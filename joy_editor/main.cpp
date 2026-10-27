// ============================================================================
//  joy_editor -- joy 2D 引擎编辑器界面
//
//  复用 joy 的即时模式 UI 层（jui_im.c）搭建编辑器外壳：
//    - 面板 / 滚动 / 裁剪：jui_panel_* / jui_scroll_*（底层是 r2d 裁剪栈）
//    - 控件：jui_button / jui_checkbox / jui_slider（复用 jui 的 retained 实例，
//      按 id 池化）
//    - 文字：统一走 jtext_printx 的 LRU 缓存，不再由调用方创建纹理
//
//  视口的世界坐标拾取 / 平移 / 缩放交互不依赖 jui（那是场景编辑逻辑，不是 UI）。
// ============================================================================
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <joy/joy.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

// ============================ 布局常量 ============================

static const int   VIEW_W    = 1280;   // 逻辑分辨率
static const int   VIEW_H    = 800;

static const float MENU_H    = 30.0f;
static const float STATUS_H  = 24.0f;
static const float CONSOLE_H = 120.0f;
static const float ASSET_H   = 190.0f;
static const float LEFT_W    = 230.0f;
static const float RIGHT_W   = 280.0f;
static const float TITLE_H   = 22.0f;  // 面板标题栏高度
static const float PAD       = 6.0f;
static const float ROW_H     = 22.0f;

static const float MAIN_Y     = MENU_H;
static const float MAIN_H     = VIEW_H - MENU_H - ASSET_H - CONSOLE_H - STATUS_H;
static const float VIEWPORT_X = LEFT_W;
static const float VIEWPORT_W = VIEW_W - LEFT_W - RIGHT_W;
static const float INSPECT_X  = VIEW_W - RIGHT_W;
static const float ASSET_Y    = MAIN_Y + MAIN_H;
static const float CONSOLE_Y  = ASSET_Y + ASSET_H;
static const float STATUS_Y   = CONSOLE_Y + CONSOLE_H;

// 面板 id：用于控件 id 分段与滚动位置存储
enum PanelId {
    PANEL_MENU = 1, PANEL_HIER, PANEL_VIEW,
    PANEL_INSP, PANEL_ASSET, PANEL_ASSET_FILES, PANEL_CONSOLE
};

// ============================ 主题 ============================

struct Theme {
    jcolor_t bg, panel, panelTitle, border, borderLight;
    jcolor_t text, textDim, textSel;
    jcolor_t accent, accentHover, hover, sel, btn, btnDisabled;
    jcolor_t viewport, grid, gridMajor, axisX, axisY;
    jcolor_t logInfo, logWarn, logError;
};

static jcolor_t RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    jcolor_t c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
}

static const Theme TH = {
    RGB(30, 30, 30),        // bg
    RGB(37, 37, 38),        // panel
    RGB(45, 45, 48),        // panelTitle
    RGB(62, 62, 66),        // border
    RGB(90, 90, 96),        // borderLight
    RGB(212, 212, 212),     // text
    RGB(133, 133, 133),     // textDim
    RGB(255, 255, 255),     // textSel
    RGB(14, 99, 156),       // accent
    RGB(23, 124, 189),      // accentHover
    RGB(50, 50, 54),        // hover
    RGB(9, 71, 113),        // sel
    RGB(51, 51, 55),        // btn
    RGB(45, 45, 48),        // btnDisabled
    RGB(21, 21, 22),        // viewport
    RGB(42, 42, 44),        // grid
    RGB(62, 62, 66),        // gridMajor
    RGB(150, 60, 60),       // axisX
    RGB(70, 140, 75),       // axisY
    RGB(180, 180, 180),     // logInfo
    RGB(220, 180, 60),      // logWarn
    RGB(230, 90, 90),       // logError
};

// ============================ UI 上下文（复用 jui_im 层） ============================

static jui_ctx_p  g_ui   = nullptr;
static jwindow_p  g_win  = nullptr;
static jr2d_p     g_r2d  = nullptr;
static font_p     g_font = nullptr;   // 编辑器统一字体（simhei 12px）

static SDL_Renderer* sdlr() { return (SDL_Renderer*)r2d_unwrap(g_r2d); }

static jrect_t RECT(float x, float y, float w, float h) {
    jrect_t r; r.x = x; r.y = y; r.w = w; r.h = h; return r;
}
static vec2_t V2(float x, float y) { vec2_t v; v.x = x; v.y = y; return v; }

// 绘制/文本/滚动/面板/控件全部委托给 jui_im 层（见 jui_im.c）。
// 下面保留几个薄封装，让各面板代码更短。

static void fill_rect(const jrect_t& r, jcolor_t c) { jui_fill_rect(g_ui, r, c); }
static void stroke_rect(const jrect_t& r, jcolor_t c) { jui_stroke_rect(g_ui, r, c); }
static void draw_line(float x1, float y1, float x2, float y2, jcolor_t c) {
    jui_line(g_ui, x1, y1, x2, y2, c);
}

// 旋转矩形：r2d 的 shape_draw_rectangle 不支持旋转，改用多边形
static void fill_rect_rot(float cx, float cy, float w, float h, float rot, jcolor_t c) {
    float cs = cosf(rot), sn = sinf(rot);
    float hw = w * 0.5f, hh = h * 0.5f;
    const float lx[4] = { -hw,  hw,  hw, -hw };
    const float ly[4] = { -hh, -hh,  hh,  hh };
    vec2_t p[5];
    for (int i = 0; i < 4; i++) {
        p[i].x = cx + lx[i] * cs - ly[i] * sn;
        p[i].y = cy + lx[i] * sn + ly[i] * cs;
    }
    p[4] = p[0];   // 闭合点：draw_polygon_filled 现在按全部 point_count 取顶点，闭合点不再必需，保留无害
    r2d_set_color(g_r2d, c.r, c.g, c.b, c.a);
    shape_draw_polygon(g_r2d, "fill", p, 5, c);
}

static int text_draw(float x, float y, const char* s, jcolor_t c) {
    return jui_text(g_ui, x, y, s, c);
}
static int text_centered(const jrect_t& r, const char* s, jcolor_t c) {
    return jui_text_centered(g_ui, r, s, c);
}
static int text_measure(const char* s, jcolor_t c) {
    int w = 0, h = 0;
    jui_text_measure(g_ui, s, c, &w, &h);
    return w;
}

// ============================ 编辑器数据模型 ============================

enum ShapeKind { SHAPE_RECT = 0, SHAPE_CIRCLE = 1, SHAPE_TRI = 2 };

static const char* shape_name(int k) {
    return k == SHAPE_RECT ? "矩形" : (k == SHAPE_CIRCLE ? "圆形" : "三角形");
}

struct Entity {
    int             id = 0;
    std::string     name;
    std::string     tag;
    int             parent = -1;
    std::vector<int> children;

    float    x = 0, y = 0;
    float    sx = 1.0f, sy = 1.0f;
    float    rot = 0.0f;
    jcolor_t color{};
    int      shape = SHAPE_RECT;
    float    w = 40.0f, h = 40.0f;
    float    radius = 22.0f;
    bool     visible = true;
    bool     expanded = true;
};

struct ViewCam { float x = 0.0f, y = 0.0f, zoom = 1.0f; };

struct LogLine { std::string text; int level; };

// ============================ 地图编辑数据模型（2D / 3D 双窗口共享） ============================

// 与游戏侧 (project_proxy3d pathfinding_component.h) 完全一致的网格约定：
//   20x20 网格, 格子索引 = gz*MAP_N + gx; 世界坐标 gx = c-10, gz = r-10 (1m/格)
#ifndef MAP_EDIT_N
#define MAP_EDIT_N 20
#endif

// 格子类型: 字符编码与 level_maps.h / 游戏 ASCII 互通
enum TileEdit {
    TILE_EMPTY = '.',   // 空地
    TILE_WALL  = '#',   // 墙
    TILE_P     = 'P',   // 玩家出生点(严格1个)
    TILE_E     = 'E',   // 敌人出生点(可多个)
    TILE_X     = 'X',   // 撤离点(严格1个)
};

// 2D/3D 共用的地图编辑网格。双窗口读写同一份, 任一修改即时同步到另一个视图。
static char g_map[MAP_EDIT_N][MAP_EDIT_N];

// 地图默认初始化: 外圈墙 + 内部空地, 并放一个 P(玩家)。
static void map_init_default() {
    for (int r = 0; r < MAP_EDIT_N; r++) {
        for (int c = 0; c < MAP_EDIT_N; c++) {
            bool border = (r == 0 || c == 0 || r == MAP_EDIT_N - 1 || c == MAP_EDIT_N - 1);
            g_map[r][c] = border ? (char)TILE_WALL : (char)TILE_EMPTY;
        }
    }
    // 默认玩家出生点居中, 留空给编辑器/游戏主场景调整
    g_map[MAP_EDIT_N / 2][MAP_EDIT_N / 2] = (char)TILE_P;
}

// 当前画刷工具(2D/3D 双窗口共用): 1=刷墙/删墙, 2=玩家P, 3=敌人E, 4=撤离X。
// 声明放在数据模型区, 供 draw_status_bar(3D 区块之前) 读取。
static int g_3dTool = 1;

// 格子 -> 世界坐标(中心)。行 r -> 世界 z, 列 c -> 世界 x。与游戏一致。
static void map_cell_to_world(int c, int r, float* wx, float* wz) {
    *wx = (float)(c - MAP_EDIT_N / 2);
    *wz = (float)(r - MAP_EDIT_N / 2);
}
// 世界坐标 -> 格子(取整)。
static void map_world_to_cell(float wx, float wz, int* c, int* r) {
    *c = (int)floorf(wx + (float)MAP_EDIT_N / 2.0f);
    *r = (int)floorf(wz + (float)MAP_EDIT_N / 2.0f);
}

// 统计校验: 返回 0=OK, 否则返回错误原因字符串。
static const char* map_validate() {
    int p = 0, x = 0, e = 0;
    for (int r = 0; r < MAP_EDIT_N; r++)
        for (int c = 0; c < MAP_EDIT_N; c++) {
            char ch = g_map[r][c];
            if (ch == (char)TILE_P) p++;
            else if (ch == (char)TILE_X) x++;
            else if (ch == (char)TILE_E) e++;
        }
    if (p != 1) return "玩家出生点 P 必须有且仅有 1 个";
    if (x != 1) return "撤离点 X 必须有且仅有 1 个";
    if (e < 1)  return "至少要有 1 个敌人出生点 E";
    // 外圈必须全墙
    for (int i = 0; i < MAP_EDIT_N; i++) {
        if (g_map[0][i] != (char)TILE_WALL) return "外圈第0行有缺口";
        if (g_map[MAP_EDIT_N-1][i] != (char)TILE_WALL) return "外圈末行有缺口";
        if (g_map[i][0] != (char)TILE_WALL) return "外圈第0列有缺口";
        if (g_map[i][MAP_EDIT_N-1] != (char)TILE_WALL) return "外圈末列有缺口";
    }
    return nullptr;
}

// 导出为 ASCII 文本(模型A): 每个格子一个字符, \n 分隔, 外圈已含墙。
// buffer 应 >= MAP_EDIT_N*(MAP_EDIT_N+1)+1 字节。
static void map_export_ascii(char* buf, int bufSize) {
    int w = 0;
    for (int r = 0; r < MAP_EDIT_N; r++) {
        for (int c = 0; c < MAP_EDIT_N; c++) {
            if (w < bufSize - 1) buf[w++] = g_map[r][c];
        }
        if (w < bufSize - 1) buf[w++] = '\n';
    }
    if (w < bufSize) buf[w] = '\0';
}

// 从 ASCII 文本加载(模型A): 逐行读取, 越界忽略, 缺行保持原样。
static void map_load_ascii(const char* src) {
    map_init_default(); // 先恢复默认, 避免残留
    int r = 0, c = 0;
    for (size_t i = 0; src[i] != '\0' && r < MAP_EDIT_N; i++) {
        char ch = src[i];
        if (ch == '\n') { r++; c = 0; continue; }
        if (c >= MAP_EDIT_N) continue;
        char v = (char)TILE_EMPTY;
        if (ch == (char)TILE_WALL) v = ch;
        else if (ch == (char)TILE_P) v = ch;
        else if (ch == (char)TILE_E) v = ch;
        else if (ch == (char)TILE_X) v = ch;
        g_map[r][c++] = v;
    }
}

// 计算当前编辑地图用来渲染的辅助: 网格是否显示/编辑器当前用哪类标记。由各窗口自行用。
unsigned long map_hash() {
    unsigned long h = 2166136261u;
    for (int r = 0; r < MAP_EDIT_N; r++)
        for (int c = 0; c < MAP_EDIT_N; c++) {
            char ch = g_map[r][c];
            h = (h ^ (unsigned char)ch) * 16777619u;
        }
    return h;
}

static std::vector<Entity>  g_ents;
static int                  g_nextEntId = 1;
static int                  g_selected  = -1;
static ViewCam              g_cam;
static std::vector<LogLine> g_log;
static bool                 g_showGrid  = true;
static bool                 g_showStats = true;
static float                g_fps       = 60.0f;
static std::string          g_statusMsg = "就绪";
static int                  g_openMenu  = -1;

// 视口拖拽状态
static int   g_dragEnt = -1;
static float g_dragDx  = 0.0f;
static float g_dragDy  = 0.0f;

static void con_log(int level, const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    LogLine l; l.text = buf; l.level = level;
    g_log.push_back(l);
    if (g_log.size() > 400) g_log.erase(g_log.begin());
}

static int ent_index(int id) {
    for (int i = 0; i < (int)g_ents.size(); i++)
        if (g_ents[i].id == id) return i;
    return -1;
}

static int ent_add(const char* name, int parent, ShapeKind shape) {
    Entity e;
    e.id     = g_nextEntId++;
    e.name   = name;
    e.tag    = "Untagged";
    e.parent = parent;
    e.shape  = shape;
    e.color  = RGB((uint8_t)(90 + rand() % 140),
                   (uint8_t)(90 + rand() % 140),
                   (uint8_t)(90 + rand() % 140));
    e.x      = (float)((rand() % 400) - 200);
    e.y      = (float)((rand() % 260) - 130);
    g_ents.push_back(e);
    if (parent >= 0) {
        int pi = ent_index(parent);
        if (pi >= 0) g_ents[pi].children.push_back(e.id);
    }
    return e.id;
}

static void ent_remove(int id) {
    int i = ent_index(id);
    if (i < 0) return;
    std::vector<int> kids = g_ents[i].children;   // 先拷贝，递归会改动数组
    for (int k : kids) ent_remove(k);

    i = ent_index(id);
    if (i < 0) return;
    if (g_ents[i].parent >= 0) {
        int pi = ent_index(g_ents[i].parent);
        if (pi >= 0) {
            auto& v = g_ents[pi].children;
            v.erase(std::remove(v.begin(), v.end(), id), v.end());
        }
    }
    g_ents.erase(g_ents.begin() + i);
    if (g_selected == id) g_selected = -1;
}

static void scene_reset() {
    g_ents.clear();
    g_nextEntId = 1;
    g_selected  = -1;
    g_cam       = ViewCam();

    int root = ent_add("Scene", -1, SHAPE_RECT);
    { int i = ent_index(root); g_ents[i].w = 0; g_ents[i].h = 0; g_ents[i].tag = "Root"; }

    int player = ent_add("Player", root, SHAPE_RECT);
    {
        int i = ent_index(player);
        g_ents[i].x = -90; g_ents[i].y = -40;
        g_ents[i].w = 46;  g_ents[i].h = 62;
        g_ents[i].tag = "Player";
        g_ents[i].color = RGB(76, 150, 220);
    }

    int ground = ent_add("Ground", root, SHAPE_RECT);
    {
        int i = ent_index(ground);
        g_ents[i].x = 0;   g_ents[i].y = 100;
        g_ents[i].w = 420; g_ents[i].h = 28;
        g_ents[i].tag = "Ground";
        g_ents[i].color = RGB(96, 96, 102);
    }

    int orb = ent_add("Orb", root, SHAPE_CIRCLE);
    {
        int i = ent_index(orb);
        g_ents[i].x = 130; g_ents[i].y = -20;
        g_ents[i].radius = 26;
        g_ents[i].tag = "Pickup";
        g_ents[i].color = RGB(226, 168, 66);
    }

    int tri = ent_add("Spikes", root, SHAPE_TRI);
    {
        int i = ent_index(tri);
        g_ents[i].x = -200; g_ents[i].y = 40;
        g_ents[i].radius = 30;
        g_ents[i].tag = "Hazard";
        g_ents[i].color = RGB(200, 84, 84);
    }

    int cam = ent_add("Camera", root, SHAPE_CIRCLE);
    {
        int i = ent_index(cam);
        g_ents[i].x = 40; g_ents[i].y = -120;
        g_ents[i].radius = 16;
        g_ents[i].tag = "Camera";
        g_ents[i].color = RGB(140, 120, 200);
    }

    con_log(0, "场景已重置：%d 个对象", (int)g_ents.size());
}

// ============================ 资源浏览器（文件系统） ============================

static const char* ASSET_ROOT = "joy_editor";

struct FsNode {
    std::string name;
    std::string path;          // 相对工作目录的路径
    bool        isDir = false;
    bool        expanded = false;
    bool        loaded = false;
    int         depth = 0;
};

static std::vector<FsNode> g_fs;
static int                 g_fsSel = 0;

struct FsScanCtx { std::vector<std::pair<std::string, bool>> items; };

static SDL_EnumerationResult SDLCALL fs_enum_cb(void* userdata,
                                                const char* dirname, const char* fname) {
    FsScanCtx* c = (FsScanCtx*)userdata;
    std::string full = std::string(dirname) + "/" + fname;
    SDL_PathInfo pi;
    bool isDir = SDL_GetPathInfo(full.c_str(), &pi) && pi.type == SDL_PATHTYPE_DIRECTORY;
    c->items.push_back(std::make_pair(std::string(fname), isDir));
    return SDL_ENUM_CONTINUE;
}

static void fs_scan(const char* dirPath, int depth, std::vector<FsNode>& out) {
    FsScanCtx ctx;
    if (!SDL_EnumerateDirectory(dirPath, fs_enum_cb, &ctx)) return;

    std::sort(ctx.items.begin(), ctx.items.end(),
        [](const std::pair<std::string, bool>& a, const std::pair<std::string, bool>& b) {
            if (a.second != b.second) return a.second;      // 目录优先
            return a.first < b.first;
        });

    for (size_t i = 0; i < ctx.items.size(); i++) {
        FsNode n;
        n.name  = ctx.items[i].first;
        n.path  = std::string(dirPath) + "/" + ctx.items[i].first;
        n.isDir = ctx.items[i].second;
        n.depth = depth;
        out.push_back(n);
    }
}

static void fs_load_children(int idx) {
    if (idx < 0 || idx >= (int)g_fs.size()) return;
    if (g_fs[idx].loaded || !g_fs[idx].isDir) return;
    std::vector<FsNode> kids;
    fs_scan(g_fs[idx].path.c_str(), g_fs[idx].depth + 1, kids);
    g_fs.insert(g_fs.begin() + idx + 1, kids.begin(), kids.end());
    g_fs[idx].loaded = true;
}

static void fs_init() {
    g_fs.clear();
    FsNode root;
    root.name = ASSET_ROOT; root.path = ASSET_ROOT;
    root.isDir = true; root.expanded = true; root.depth = 0;
    g_fs.push_back(root);
    fs_load_children(0);
    g_fsSel = 0;
}

static jcolor_t file_color(const std::string& name) {
    auto has = [&](const char* ext) {
        size_t el = SDL_strlen(ext);
        return name.size() >= el &&
               SDL_strcasecmp(name.c_str() + name.size() - el, ext) == 0;
    };
    if (has(".png") || has(".jpg") || has(".bmp"))            return RGB(96, 170, 120);
    if (has(".ttf") || has(".TTF"))                           return RGB(216, 146, 72);
    if (has(".tsx") || has(".tmx"))                           return RGB(92, 156, 214);
    if (has(".spv") || has(".shader") || has(".glsl"))        return RGB(176, 122, 208);
    if (has(".wav") || has(".mp3") || has(".ogg"))            return RGB(214, 110, 140);
    return RGB(140, 140, 146);
}

// ============================ 菜单栏 ============================

struct MenuDef { const char* title; const char* items[6]; int n; };

static const MenuDef MENUS[] = {
    { "文件", { "新建场景", "打开...", "保存", "退出" }, 4 },
    { "编辑", { "添加对象", "复制", "删除", "全部展开", "全部折叠" }, 5 },
    { "视图", { "重置视图", "切换网格", "适应窗口", "切换统计" }, 4 },
    { "帮助", { "关于 joy" }, 1 },
};
static const int MENU_COUNT = 4;

static void menu_action(const char* act) {
    if (SDL_strcmp(act, "新建场景") == 0) {
        scene_reset();
    } else if (SDL_strcmp(act, "退出") == 0) {
        push_quit_event();
    } else if (SDL_strcmp(act, "添加对象") == 0) {
        int p = g_selected >= 0 ? g_selected : (g_ents.empty() ? -1 : g_ents[0].id);
        int nid = ent_add("NewObject", p, SHAPE_RECT);
        g_selected = nid;
        con_log(0, "已添加对象 #%d", nid);
    } else if (SDL_strcmp(act, "删除") == 0) {
        if (g_selected >= 0) { con_log(1, "已删除对象 #%d", g_selected); ent_remove(g_selected); }
        else con_log(1, "没有选中对象");
    } else if (SDL_strcmp(act, "复制") == 0) {
        int i = ent_index(g_selected);
        if (i >= 0) {
            Entity c = g_ents[i];
            c.id = g_nextEntId++;
            c.children.clear();
            c.x += 24.0f; c.y += 24.0f;
            c.name += "_copy";
            g_ents.push_back(c);
            int pi = ent_index(g_ents[i].id);
            if (pi >= 0) g_ents[pi].children.push_back(c.id);
            g_selected = c.id;
            con_log(0, "已复制为 #%d", c.id);
        } else {
            con_log(1, "没有选中对象");
        }
    } else if (SDL_strcmp(act, "全部展开") == 0) {
        for (auto& e : g_ents) e.expanded = true;
    } else if (SDL_strcmp(act, "全部折叠") == 0) {
        for (auto& e : g_ents) e.expanded = false;
    } else if (SDL_strcmp(act, "重置视图") == 0) {
        g_cam = ViewCam();
        con_log(0, "视图已重置");
    } else if (SDL_strcmp(act, "切换网格") == 0) {
        g_showGrid = !g_showGrid;
        con_log(0, "网格 %s", g_showGrid ? "开" : "关");
    } else if (SDL_strcmp(act, "适应窗口") == 0) {
        g_cam = ViewCam();
    } else if (SDL_strcmp(act, "切换统计") == 0) {
        g_showStats = !g_showStats;
    } else if (SDL_strcmp(act, "关于 joy") == 0) {
        con_log(0, "joy 2D 引擎编辑器 — r2d 渲染后端 / 即时模式 UI 层");
    } else {
        con_log(2, "菜单项 [%s] 尚未实现", act);
    }
}

static void draw_menu_bar() {
    jrect_t bar = RECT(0, 0, VIEW_W, MENU_H);
    fill_rect(bar, TH.panelTitle);
    draw_line(0, MENU_H - 1.0f, VIEW_W, MENU_H - 1.0f, TH.border);

    static int menuSeq = 0;
    float x = 8.0f;
    for (int m = 0; m < MENU_COUNT; m++) {
        int tw = text_measure(MENUS[m].title, TH.text);
        jrect_t r = RECT(x, 3.0f, tw + 18.0f, MENU_H - 7.0f);

        int  id = PANEL_MENU * 1000 + (++menuSeq);
        bool hovered = jui_hit(g_ui, r);
        if (hovered && jui_is_pressed(g_ui)) {
            g_openMenu = (g_openMenu == m) ? -1 : m;
        }
        bool isOpen = (g_openMenu == m);
        if (isOpen || hovered) fill_rect(r, isOpen ? TH.sel : TH.hover);
        text_centered(r, MENUS[m].title, TH.text);

        if (isOpen) {
            float   pw = 168.0f;
            float   ph = MENUS[m].n * ROW_H + 6.0f;
            jrect_t panel = RECT(r.x, MENU_H, pw, ph);
            fill_rect(panel, TH.panel);
            stroke_rect(panel, TH.borderLight);

            bool acted = false;
            for (int i = 0; i < MENUS[m].n && !acted; i++) {
                jrect_t ir = RECT(panel.x + 3.0f, panel.y + 3.0f + i * ROW_H,
                                  pw - 6.0f, ROW_H);
                if (jui_button(g_ui, id * 100 + i, ir, MENUS[m].items[i])) {
                    g_openMenu = -1;
                    menu_action(MENUS[m].items[i]);
                    acted = true;
                }
            }
            if (jui_is_pressed(g_ui) && !jui_hit(g_ui, panel) && !jui_hit(g_ui, r))
                g_openMenu = -1;
            if (acted) { menuSeq = 0; return; }
        }
        x += r.w + 2.0f;
    }
    menuSeq = 0;

    const char* title = "joy_editor";
    int tw2 = text_measure(title, TH.textDim);
    text_draw(VIEW_W - tw2 - 10.0f, 9.0f, title, TH.textDim);
}

// ============================ 层级树 ============================

static void hier_count_rows(int id, int* rows) {
    int i = ent_index(id);
    if (i < 0) return;
    (*rows)++;
    if (g_ents[i].expanded) {
        for (int c : g_ents[i].children) hier_count_rows(c, rows);
    }
}

static void hier_row(int entId, int depth, float x, float* y, float w,
                     float clipTop, float clipBottom, int* picked) {
    int i = ent_index(entId);
    if (i < 0) return;
    Entity& e = g_ents[i];

    jrect_t r = RECT(x + depth * 14.0f, *y, w - depth * 14.0f, ROW_H);
    *y += ROW_H;

    bool culled = (r.y + r.h < clipTop) || (r.y > clipBottom);
    if (!culled) {
        int  id      = PANEL_HIER * 1000 + entId;
        bool hovered = jui_hit(g_ui, r);
        bool isSel   = (g_selected == entId);

        if (isSel)        fill_rect(r, TH.sel);
        else if (hovered) fill_rect(r, TH.hover);

        if (!e.children.empty()) {
            jrect_t ar = RECT(r.x + 2.0f, r.y + 4.0f, 14.0f, 14.0f);
            if (jui_button(g_ui, id + 1000000, ar, e.expanded ? "v" : ">"))
                e.expanded = !e.expanded;
        }

        fill_rect(RECT(r.x + 18.0f, r.y + 6.0f, 10.0f, 10.0f), e.color);
        stroke_rect(RECT(r.x + 18.0f, r.y + 6.0f, 10.0f, 10.0f), TH.border);
        text_draw(r.x + 34.0f, r.y + 4.0f, e.name.c_str(), isSel ? TH.textSel : TH.text);

        if (hovered && jui_is_pressed(g_ui) && *picked < 0) {
            g_selected = entId;
            *picked = entId;
        }
    }

    if (e.expanded) {
        for (int c : e.children) hier_row(c, depth + 1, x, y, w, clipTop, clipBottom, picked);
    }
}

static void draw_hierarchy(const jrect_t& panel) {
    int rows = 0;
    if (!g_ents.empty()) hier_count_rows(g_ents[0].id, &rows);

    jrect_t view = RECT(panel.x + PAD, panel.y + TITLE_H + PAD,
                        panel.w - PAD * 2.0f, panel.h - TITLE_H - PAD * 2.0f);
    float contentH = rows * ROW_H + 4.0f;
    float off      = jui_scroll_begin(g_ui, PANEL_HIER, view, contentH);

    jui_push_clip(g_ui, view);
    float y = view.y - off;
    int   picked = -1;
    if (!g_ents.empty())
        hier_row(g_ents[0].id, 0, view.x, &y, view.w,
                 view.y, view.y + view.h, &picked);
    jui_pop_clip(g_ui);

    jui_scrollbar(g_ui, view, contentH, off);
    stroke_rect(view, TH.border);
}

// ============================ 视口 ============================

static float w2sx(float wx, const jrect_t& vp) {
    return (wx - g_cam.x) * g_cam.zoom + vp.x + vp.w * 0.5f;
}
static float w2sy(float wy, const jrect_t& vp) {
    return (wy - g_cam.y) * g_cam.zoom + vp.y + vp.h * 0.5f;
}
static float s2wx(float sx, const jrect_t& vp) {
    return (sx - vp.x - vp.w * 0.5f) / g_cam.zoom + g_cam.x;
}
static float s2wy(float sy, const jrect_t& vp) {
    return (sy - vp.y - vp.h * 0.5f) / g_cam.zoom + g_cam.y;
}

static bool tri_hit(float lx, float ly, float r) {
    if (ly < -r || ly > r * 0.6f) return false;
    float t         = (ly + r) / (r * 1.6f);
    float halfWidth = r * 0.92f * t;
    return fabsf(lx) <= halfWidth;
}

static bool ent_hit(const Entity& e, float wx, float wy) {
    if (e.shape == SHAPE_RECT && e.w <= 0.0f && e.h <= 0.0f) return false;

    float dx = wx - e.x, dy = wy - e.y;
    float cs = cosf(-e.rot), sn = sinf(-e.rot);
    float lx = dx * cs - dy * sn;
    float ly = dx * sn + dy * cs;

    if (e.shape == SHAPE_CIRCLE) {
        float r = e.radius * e.sx;
        return (lx * lx + ly * ly) <= (r * r);
    }
    if (e.shape == SHAPE_TRI) {
        return tri_hit(lx, ly, e.radius * e.sx);
    }
    return fabsf(lx) <= e.w * 0.5f * e.sx && fabsf(ly) <= e.h * 0.5f * e.sy;
}

static void draw_viewport(const jrect_t& panel) {
    jrect_t vp = RECT(panel.x + 1.0f, panel.y + TITLE_H + 1.0f,
                      panel.w - 2.0f, panel.h - TITLE_H - 2.0f);

    jui_push_clip(g_ui, vp);
    fill_rect(vp, TH.viewport);

    // ---- 网格 ----
    if (g_showGrid) {
        float step = 32.0f;
        while (step * g_cam.zoom < 14.0f) step *= 2.0f;
        float x0 = s2wx(vp.x, vp), x1 = s2wx(vp.x + vp.w, vp);
        float y0 = s2wy(vp.y, vp), y1 = s2wy(vp.y + vp.h, vp);

        for (float gx = floorf(x0 / step) * step; gx <= x1; gx += step) {
            bool major = fabsf(gx - roundf(gx / (step * 5.0f)) * step * 5.0f) < 0.01f;
            draw_line(w2sx(gx, vp), vp.y, w2sx(gx, vp), vp.y + vp.h,
                      major ? TH.gridMajor : TH.grid);
        }
        for (float gy = floorf(y0 / step) * step; gy <= y1; gy += step) {
            bool major = fabsf(gy - roundf(gy / (step * 5.0f)) * step * 5.0f) < 0.01f;
            draw_line(vp.x, w2sy(gy, vp), vp.x + vp.w, w2sy(gy, vp),
                      major ? TH.gridMajor : TH.grid);
        }
        draw_line(w2sx(0, vp), vp.y, w2sx(0, vp), vp.y + vp.h, TH.axisX);
        draw_line(vp.x, w2sy(0, vp), vp.x + vp.w, w2sy(0, vp), TH.axisY);
    }

    // ---- 对象 ----
    for (const Entity& e : g_ents) {
        if (!e.visible) continue;
        if (e.shape == SHAPE_RECT && e.w <= 0.0f && e.h <= 0.0f) continue;

        float px = w2sx(e.x, vp), py = w2sy(e.y, vp);
        r2d_set_color(g_r2d, e.color.r, e.color.g, e.color.b, e.color.a);

        if (e.shape == SHAPE_CIRCLE) {
            vec2_t c = V2(px, py);
            shape_draw_circle(g_r2d, "fill", c, e.radius * e.sx * g_cam.zoom, 24);
        } else if (e.shape == SHAPE_TRI) {
            float r = e.radius * e.sx * g_cam.zoom;
            vec2_t p[4] = {
                V2(px, py - r), V2(px + r * 0.92f, py + r * 0.6f),
                V2(px - r * 0.92f, py + r * 0.6f), V2(px, py - r)
            };
            shape_draw_polygon(g_r2d, "fill", p, 4, e.color);
        } else {
            fill_rect_rot(px, py, e.w * e.sx * g_cam.zoom,
                          e.h * e.sy * g_cam.zoom, e.rot, e.color);
        }
    }

    // ---- 选中框 + 四角手柄 ----
    int si = ent_index(g_selected);
    if (si >= 0) {
        const Entity& e = g_ents[si];
        if (!(e.shape == SHAPE_RECT && e.w <= 0.0f && e.h <= 0.0f)) {
            float hw, hh;
            if (e.shape == SHAPE_CIRCLE) { hw = e.radius * e.sx; hh = e.radius * e.sy; }
            else if (e.shape == SHAPE_TRI) { hw = e.radius * 0.92f * e.sx; hh = e.radius * 0.8f * e.sy; }
            else { hw = e.w * 0.5f * e.sx; hh = e.h * 0.5f * e.sy; }
            hw += 5.0f; hh += 5.0f;

            float px = w2sx(e.x, vp), py = w2sy(e.y, vp);
            float bx = px - hw * g_cam.zoom, by = py - hh * g_cam.zoom;
            float bw = hw * 2.0f * g_cam.zoom, bh = hh * 2.0f * g_cam.zoom;
            stroke_rect(RECT(bx, by, bw, bh), TH.accentHover);

            const float hs = 4.0f;
            jrect_t h[4] = {
                RECT(bx - hs,          by - hs,          hs * 2, hs * 2),
                RECT(bx + bw - hs,     by - hs,          hs * 2, hs * 2),
                RECT(bx - hs,          by + bh - hs,     hs * 2, hs * 2),
                RECT(bx + bw - hs,     by + bh - hs,     hs * 2, hs * 2)
            };
            for (int k = 0; k < 4; k++) fill_rect(h[k], TH.accentHover);
        }
    }
    jui_pop_clip(g_ui);

    // ---- 交互 ----
    float mx, my;
    jui_get_mouse(g_ui, &mx, &my);
    bool overVp = jui_hit(g_ui, vp);

    // 滚轮缩放，以光标为锚点
    float wheel = jui_take_wheel(g_ui);
    if (overVp && wheel != 0.0f) {
        float wxBefore = s2wx(mx, vp), wyBefore = s2wy(my, vp);
        g_cam.zoom = std::clamp(g_cam.zoom * (wheel > 0 ? 1.12f : 1.0f / 1.12f),
                                0.15f, 8.0f);
        g_cam.x += wxBefore - s2wx(mx, vp);
        g_cam.y += wyBefore - s2wy(my, vp);
    }

    // 中键 / 右键拖拽平移：这里直接读 SDL 按钮状态（jui_im 只跟踪左键）
    static bool  panning = false;
    static float panSx = 0, panSy = 0, panCx = 0, panCy = 0;
    bool middleDown = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MIDDLE) != 0;
    bool rightDown  = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_RIGHT)  != 0;
    if (overVp && (middleDown || rightDown) && !panning) {
        panning = true; panSx = mx; panSy = my;
        panCx = g_cam.x; panCy = g_cam.y;
    }
    if (panning) {
        g_cam.x = panCx - (mx - panSx) / g_cam.zoom;
        g_cam.y = panCy - (my - panSy) / g_cam.zoom;
        if (!middleDown && !rightDown) panning = false;
    }

    // 左键拾取 / 拖拽对象
    if (overVp && jui_is_pressed(g_ui) && !panning) {
        float wx = s2wx(mx, vp), wy = s2wy(my, vp);
        int hitId = -1;
        for (int i = (int)g_ents.size() - 1; i >= 0; i--) {
            if (ent_hit(g_ents[i], wx, wy)) { hitId = g_ents[i].id; break; }
        }
        g_selected = hitId;
        if (hitId >= 0) {
            int i = ent_index(hitId);
            g_dragEnt = hitId;
            g_dragDx  = g_ents[i].x - wx;
            g_dragDy  = g_ents[i].y - wy;
        }
    }
    if (g_dragEnt >= 0) {
        int i = ent_index(g_dragEnt);
        if (i >= 0 && jui_is_down(g_ui)) {
            float wx = s2wx(mx, vp), wy = s2wy(my, vp);
            float nx = wx + g_dragDx, ny = wy + g_dragDy;
            if (jui_is_ctrl(g_ui)) {
                nx = roundf(nx / 16.0f) * 16.0f;
                ny = roundf(ny / 16.0f) * 16.0f;
            }
            g_ents[i].x = nx;
            g_ents[i].y = ny;
        } else {
            g_dragEnt = -1;
        }
    }

    // ---- 左上角 HUD ----
    if (g_showStats) {
        char buf[128];
        SDL_snprintf(buf, sizeof(buf), "缩放 %d%%   视点 (%d, %d)",
                     (int)(g_cam.zoom * 100.0f), (int)g_cam.x, (int)g_cam.y);
        fill_rect(RECT(vp.x + 8.0f, vp.y + 8.0f, 210.0f, 40.0f), RGB(0, 0, 0, 120));
        text_draw(vp.x + 14.0f, vp.y + 12.0f, buf, TH.textDim);
        text_draw(vp.x + 14.0f, vp.y + 28.0f, "中键/右键拖拽平移 · 滚轮缩放",
                  RGB(110, 110, 116));
    }

    stroke_rect(vp, TH.border);
}

// ============================ 检查器 ============================

static void kv_row(float x, float y, float w, const char* k, const char* val, jcolor_t vc) {
    text_draw(x, y + 4.0f, k, TH.textDim);
    int vw = text_measure(val, vc);
    text_draw(x + w - vw, y + 4.0f, val, vc);
}

// 带标签与数值的滑块行
static bool slider_row(float x, float y, float w, const char* label, int id,
                       float* v, float minv, float maxv, const char* fmt) {
    text_draw(x, y + 4.0f, label, TH.textDim);
    jrect_t track = RECT(x + 78.0f, y + 8.0f, w - 78.0f - 52.0f, 6.0f);
    bool changed = jui_slider(g_ui, id, track, v, minv, maxv);
    char buf[64];
    SDL_snprintf(buf, sizeof(buf), fmt, *v);
    text_draw(x + w - 48.0f, y + 4.0f, buf, TH.text);
    return changed;
}

static void draw_inspector(const jrect_t& panel) {
    jrect_t view = RECT(panel.x + PAD, panel.y + TITLE_H + PAD,
                        panel.w - PAD * 2.0f, panel.h - TITLE_H - PAD * 2.0f);

    int si = ent_index(g_selected);
    if (si < 0) {
        text_draw(view.x, view.y + 4.0f, "未选中对象", TH.textDim);
        text_draw(view.x, view.y + 24.0f, "在视口或层级树中点选一个对象",
                  RGB(105, 105, 110));
        return;
    }
    Entity& e = g_ents[si];

    const float contentH = 6.0f * ROW_H + 8.0f
                         + (ROW_H + 2.0f * ROW_H + 3.0f * 26.0f) + 8.0f
                         + (ROW_H + 26.0f + 3.0f * 26.0f + 30.0f) + 8.0f;
    float off = jui_scroll_begin(g_ui, PANEL_INSP, view, contentH);

    jui_push_clip(g_ui, view);
    {
        float x = view.x, w = view.w - 8.0f;
        float y = view.y - off;
        char  b[64];

        text_draw(x, y + 4.0f, "对象", TH.accentHover); y += ROW_H;
        SDL_snprintf(b, sizeof(b), "#%d", e.id);
        kv_row(x, y, w, "ID",   b,                  TH.text);   y += ROW_H;
        kv_row(x, y, w, "名称", e.name.c_str(),     TH.text);   y += ROW_H;
        kv_row(x, y, w, "标签", e.tag.c_str(),      TH.text);   y += ROW_H;
        kv_row(x, y, w, "形状", shape_name(e.shape),TH.text);   y += ROW_H;

        y += 8.0f;
        text_draw(x, y + 4.0f, "变换", TH.accentHover); y += ROW_H;
        SDL_snprintf(b, sizeof(b), "%.1f", e.x);
        kv_row(x, y, w, "位置 X", b, TH.text); y += ROW_H;
        SDL_snprintf(b, sizeof(b), "%.1f", e.y);
        kv_row(x, y, w, "位置 Y", b, TH.text); y += ROW_H;

        slider_row(x, y, w, "缩放 X", PANEL_INSP * 100 + 1, &e.sx, 0.1f, 3.0f, "%.2f");
        y += 26.0f;
        slider_row(x, y, w, "缩放 Y", PANEL_INSP * 100 + 2, &e.sy, 0.1f, 3.0f, "%.2f");
        y += 26.0f;

        float deg = e.rot * 180.0f / 3.14159265f;
        if (slider_row(x, y, w, "旋转", PANEL_INSP * 100 + 3, &deg,
                       -180.0f, 180.0f, "%.0f°")) {
            e.rot = deg * 3.14159265f / 180.0f;
        }
        y += 26.0f;

        y += 8.0f;
        text_draw(x, y + 4.0f, "外观", TH.accentHover); y += ROW_H;
        if (jui_checkbox(g_ui, PANEL_INSP * 100 + 4,
                         RECT(x, y, w, 22.0f), &e.visible, "可见")) { }
        y += 26.0f;

        float ch[3] = { (float)e.color.r, (float)e.color.g, (float)e.color.b };
        const char* chName[3] = { "红", "绿", "蓝" };
        for (int c = 0; c < 3; c++) {
            slider_row(x, y, w, chName[c], PANEL_INSP * 100 + 5 + c,
                       &ch[c], 0.0f, 255.0f, "%.0f");
            y += 26.0f;
        }
        e.color.r = (uint8_t)ch[0];
        e.color.g = (uint8_t)ch[1];
        e.color.b = (uint8_t)ch[2];

        fill_rect(RECT(x, y, 46.0f, 22.0f), e.color);
        stroke_rect(RECT(x, y, 46.0f, 22.0f), TH.borderLight);
        text_draw(x + 54.0f, y + 4.0f, "颜色预览", TH.textDim);
    }
    jui_pop_clip(g_ui);

    jui_scrollbar(g_ui, view, contentH, off);
}

// ============================ 资源浏览器 ============================

struct FileItem { std::string name; bool isDir; };

static void draw_assets(const jrect_t& panel) {
    jrect_t view = RECT(panel.x + PAD, panel.y + TITLE_H + PAD + 16.0f,
                        panel.w - PAD * 2.0f, panel.h - TITLE_H - PAD * 2.0f - 16.0f);

    const float treeW = 260.0f;
    jrect_t treeView = RECT(view.x, view.y, treeW, view.h);
    jrect_t fileView = RECT(view.x + treeW + 8.0f, view.y,
                            view.w - treeW - 8.0f, view.h);

    // ---------------- 左：目录树 ----------------
    float treeContentH = (float)g_fs.size() * ROW_H + 4.0f;
    float treeOff      = jui_scroll_begin(g_ui, PANEL_ASSET, treeView, treeContentH);

    jui_push_clip(g_ui, treeView);
    {
        float y = treeView.y - treeOff;
        for (int i = 0; i < (int)g_fs.size(); i++) {
            FsNode& n = g_fs[i];
            jrect_t r = RECT(treeView.x + n.depth * 14.0f, y,
                             treeView.w - n.depth * 14.0f, ROW_H);
            y += ROW_H;
            if (r.y + r.h < treeView.y - ROW_H || r.y > treeView.y + treeView.h + ROW_H) continue;

            bool hovered = jui_hit(g_ui, r);
            bool isSel   = (g_fsSel == i);
            if (isSel)        fill_rect(r, TH.sel);
            else if (hovered) fill_rect(r, TH.hover);

            if (n.isDir) {
                jrect_t ar = RECT(r.x + 2.0f, r.y + 4.0f, 14.0f, 14.0f);
                if (jui_button(g_ui, PANEL_ASSET * 1000 + i, ar, n.expanded ? "v" : ">")) {
                    n.expanded = !n.expanded;
                    if (n.expanded) {
                        fs_load_children(i);
                    } else {
                        int j = i + 1;
                        while (j < (int)g_fs.size() && g_fs[j].depth > n.depth) j++;
                        g_fs.erase(g_fs.begin() + i + 1, g_fs.begin() + j);
                        n.loaded = false;
                    }
                }
            }
            fill_rect(RECT(r.x + 18.0f, r.y + 6.0f, 10.0f, 10.0f),
                      n.isDir ? RGB(216, 190, 120) : file_color(n.name));
            text_draw(r.x + 34.0f, r.y + 4.0f, n.name.c_str(), isSel ? TH.textSel : TH.text);

            if (hovered && jui_is_pressed(g_ui)) {
                g_fsSel = i;
                if (n.isDir && !n.expanded) { n.expanded = true; fs_load_children(i); }
            }
        }
    }
    jui_pop_clip(g_ui);
    jui_scrollbar(g_ui, treeView, treeContentH, treeOff);
    stroke_rect(treeView, TH.border);

    // ---------------- 右：文件网格 ----------------
    std::string dirPath = (g_fsSel >= 0 && g_fsSel < (int)g_fs.size())
                          ? g_fs[g_fsSel].path : std::string(ASSET_ROOT);

    static std::vector<FileItem> files;
    static std::string           filesFor;
    if (filesFor != dirPath) {
        files.clear();
        std::vector<FsNode> raw;
        fs_scan(dirPath.c_str(), 0, raw);
        for (size_t i = 0; i < raw.size(); i++) files.push_back({ raw[i].name, raw[i].isDir });
        filesFor = dirPath;
    }

    const float cellW = 150.0f, cellH = 26.0f;
    int   cols = std::max(1, (int)(fileView.w / cellW));
    float fileContentH = (float)((files.size() + cols - 1) / cols) * cellH + 4.0f;
    float fileOff      = jui_scroll_begin(g_ui, PANEL_ASSET_FILES, fileView, fileContentH);

    fill_rect(fileView, RGB(32, 32, 34));
    jui_push_clip(g_ui, fileView);
    {
        int idx = 0;
        for (size_t i = 0; i < files.size(); i++) {
            int   col = idx % cols, row = idx / cols;
            float cx  = fileView.x + (float)col * cellW;
            float cy  = fileView.y + (float)row * cellH - fileOff;
            idx++;
            if (cy + cellH < fileView.y - cellH || cy > fileView.y + fileView.h + cellH) continue;

            jrect_t r = RECT(cx + 2.0f, cy + 1.0f, cellW - 4.0f, cellH - 2.0f);
            if (jui_hit(g_ui, r)) fill_rect(r, TH.hover);
            fill_rect(RECT(r.x + 5.0f, r.y + 5.0f, 12.0f, 12.0f),
                      files[i].isDir ? RGB(216, 190, 120) : file_color(files[i].name));

            std::string shown = files[i].name;
            if (shown.size() > 17) shown = shown.substr(0, 16) + "…";
            text_draw(r.x + 22.0f, r.y + 5.0f, shown.c_str(),
                      files[i].isDir ? RGB(205, 205, 210) : TH.text);
        }
        if (files.empty()) {
            text_draw(fileView.x + 8.0f, fileView.y + 4.0f, "（空目录）", TH.textDim);
        }
    }
    jui_pop_clip(g_ui);
    jui_scrollbar(g_ui, fileView, fileContentH, fileOff);
    stroke_rect(fileView, TH.border);

    std::string tip = dirPath;
    if (tip.size() > 70) tip = "..." + tip.substr(tip.size() - 67);
    text_draw(view.x, view.y - 15.0f, tip.c_str(), TH.textDim);
}

// ============================ 控制台 ============================

static void draw_console(const jrect_t& panel) {
    jrect_t view = RECT(panel.x + PAD, panel.y + TITLE_H + PAD,
                        panel.w - PAD * 2.0f, panel.h - TITLE_H - PAD * 2.0f);

    float contentH = (float)g_log.size() * 16.0f + 4.0f;
    float maxOff   = std::max(0.0f, contentH - view.h);

    // 贴底跟随：只有用户本来就滚到底部时才自动跟随新日志。
    // 先取当前偏移判断是否贴底，再在内容变多时把偏移拉到底。
    {
        static int  lastCount = -1;
        static bool wasAtBottom = true;
        float offNow = jui_scroll_begin(g_ui, PANEL_CONSOLE, view, contentH);
        if ((int)g_log.size() != lastCount) {
            lastCount = (int)g_log.size();
            if (wasAtBottom) jui_scroll_set_offset(g_ui, PANEL_CONSOLE, maxOff, contentH, view.h);
        } else {
            wasAtBottom = (offNow >= maxOff - 2.0f);
        }
    }
    float off = jui_scroll_begin(g_ui, PANEL_CONSOLE, view, contentH);

    jui_push_clip(g_ui, view);
    {
        for (size_t i = 0; i < g_log.size(); i++) {
            float ly = view.y - off + (float)i * 16.0f;
            if (ly + 16.0f < view.y || ly > view.y + view.h) continue;

            int level = g_log[i].level;
            const char* tag = level == 2 ? "[错误] " : (level == 1 ? "[警告] " : "[信息] ");
            jcolor_t    c   = level == 2 ? TH.logError : (level == 1 ? TH.logWarn : TH.logInfo);

            text_draw(view.x, ly + 2.0f, tag, level == 2 ? TH.logError : TH.textDim);
            int tw = text_measure(tag, TH.textDim);
            text_draw(view.x + tw, ly + 2.0f, g_log[i].text.c_str(), c);
        }
    }
    jui_pop_clip(g_ui);
    jui_scrollbar(g_ui, view, contentH, off);
}

// ============================ 状态栏 ============================

static void draw_status_bar() {
    jrect_t bar = RECT(0, STATUS_Y, VIEW_W, STATUS_H);
    fill_rect(bar, TH.panelTitle);
    draw_line(0, STATUS_Y, VIEW_W, STATUS_Y, TH.border);

    char buf[256];
    text_draw(10.0f, STATUS_Y + 5.0f, g_statusMsg.c_str(), TH.textDim);

    SDL_snprintf(buf, sizeof(buf), "对象 %d", (int)g_ents.size());
    text_draw(VIEW_W - 330.0f, STATUS_Y + 5.0f, buf, TH.textDim);

    const char* selName = "无";
    int si = ent_index(g_selected);
    if (si >= 0) selName = g_ents[si].name.c_str();
    SDL_snprintf(buf, sizeof(buf), "选中 %s", selName);
    text_draw(VIEW_W - 240.0f, STATUS_Y + 5.0f, buf, TH.textDim);

    // 地图校验状态(3D 画刷与 2D 共用同一 g_map)
    {
        const char* v = map_validate();
        const char* tooln = g_3dTool==1?"刷墙":(g_3dTool==2?"玩家P":(g_3dTool==3?"敌人E":"撤离X"));
        if (v) SDL_snprintf(buf, sizeof(buf), "地图: [%s] %s", tooln, v);
        else   SDL_snprintf(buf, sizeof(buf), "地图: [%s] 校验通过", tooln);
        text_draw(VIEW_W - 800.0f, STATUS_Y + 5.0f, buf, v ? TH.logWarn : TH.logInfo);
    }

    SDL_snprintf(buf, sizeof(buf), "%.0f FPS", g_fps);
    int w = text_measure(buf, g_fps < 30.0f ? TH.logWarn : TH.textDim);
    text_draw(VIEW_W - w - 10.0f, STATUS_Y + 5.0f, buf,
              g_fps < 30.0f ? TH.logWarn : TH.textDim);
}

// ============================ 帧渲染 ============================

static void render_frame() {
    fill_rect(RECT(0, 0, VIEW_W, VIEW_H), TH.bg);
    draw_menu_bar();

    jrect_t rHier  = RECT(0, MAIN_Y, LEFT_W, MAIN_H);
    jrect_t rView  = RECT(VIEWPORT_X, MAIN_Y, VIEWPORT_W, MAIN_H);
    jrect_t rInsp  = RECT(INSPECT_X, MAIN_Y, RIGHT_W, MAIN_H);
    jrect_t rAsset = RECT(0, ASSET_Y, VIEW_W, ASSET_H);
    jrect_t rCons  = RECT(0, CONSOLE_Y, VIEW_W, CONSOLE_H);

    jui_panel_begin(g_ui, rHier, "层级");
    draw_hierarchy(rHier);
    jui_panel_end(g_ui);
    jui_panel_frame(g_ui, rHier);

    jui_panel_begin(g_ui, rView, "视口");
    draw_viewport(rView);
    jui_panel_end(g_ui);
    jui_panel_frame(g_ui, rView);

    jui_panel_begin(g_ui, rInsp, "检查器");
    draw_inspector(rInsp);
    jui_panel_end(g_ui);
    jui_panel_frame(g_ui, rInsp);

    jui_panel_begin(g_ui, rAsset, "资源浏览器");
    draw_assets(rAsset);
    jui_panel_end(g_ui);
    jui_panel_frame(g_ui, rAsset);

    jui_panel_begin(g_ui, rCons, "控制台");
    draw_console(rCons);
    jui_panel_end(g_ui);
    jui_panel_frame(g_ui, rCons);

    draw_status_bar();
}

// ============================ 3D 画刷窗口（窗口B, r3d 后端） ============================

// 窗口B 全局状态: 3D 后端、相机、当前工具游标、当前高亮格。
static jwindow_p  g_win3d  = nullptr;   // 3D 画刷窗口
static jr3d_p     g_r3d    = nullptr;   // 3D 渲染上下文

// 3D 渲染复用的全局 mesh(在 r3d_cleanup 里统一销毁, 避免 static 跨 context 崩溃/泄漏)
static render_mesh_p g_meshFloor  = nullptr;   // 大地面平面
static render_mesh_p g_meshWall   = nullptr;   // 墙立方体
static render_mesh_p g_meshMark   = nullptr;   // 标记扁块
static render_mesh_p g_meshHover  = nullptr;   // 悬停格线框
static render_mesh_p g_meshText   = nullptr;   // 工具菜单/HUD 文字用的单位 quad

static int  g_3dActive  = 0;   // 3D 窗口是否已成功打开(0=关闭)
static int  g_3dCamYaw  = 0;   // 相机 yaw(度)
static int  g_3dCamPitch = -60; // 相机 pitch(度, 略俯视)。负=向下看。
static float g_3dCamDist = 18.0f; // 相机到地图中心距离
static float g_3dMouseX = 0, g_3dMouseY = 0;  // 3D 窗口内鼠标像素坐标
static int  g_3dHoverC = -1, g_3dHoverR = -1; // 悬停格 (上一次)
static bool g_3dPainting = false;   // 按住左键/右键连续刷

// 3D 窗口的清理
static void r3d_cleanup() {
    if (g_r3d) {
        if (g_meshFloor) { r3d_mesh_destroy(g_r3d, g_meshFloor); g_meshFloor = nullptr; }
        if (g_meshWall)  { r3d_mesh_destroy(g_r3d, g_meshWall);  g_meshWall  = nullptr; }
        if (g_meshMark)  { r3d_mesh_destroy(g_r3d, g_meshMark);  g_meshMark  = nullptr; }
        if (g_meshHover) { r3d_mesh_destroy(g_r3d, g_meshHover); g_meshHover = nullptr; }
        if (g_meshText)  { r3d_mesh_destroy(g_r3d, g_meshText);  g_meshText  = nullptr; }
        r3d_destroy(g_r3d); g_r3d = nullptr;
    }
    if (g_win3d) { destroy_window(g_win3d); g_win3d = nullptr; }
    g_3dActive = 0;
}

// 计算 3D 相机 eye/target, 基于球坐标绕地图原点。
static void r3d_camera_around(float* eye, float target[3]) {
    float yaw = g_3dCamYaw * 3.14159265f / 180.0f;
    float pit = g_3dCamPitch * 3.14159265f / 180.0f;
    float cx = cosf(pit) * sinf(yaw) * g_3dCamDist;
    float cz = cosf(pit) * cosf(yaw) * g_3dCamDist;
    float cy = sinf(pit) * g_3dCamDist;
    eye[0] = target[0] + cx;
    eye[1] = target[1] + cy;
    eye[2] = target[2] + cz;
}

// 从 3D 窗口鼠标屏幕坐标求地面拾取: 用与 render_3d_frame 完全一致的相机(绕地图中心、fov=55)
// 构造"屏幕 -> 世界射线", 再与地平面 Y=0 求交, 得到精确的 XZ 世界坐标后映射到格子。
// 不再用任何经验像素近似, 从而消除鼠标与放置盒子的偏差。
static void r3d_pick_cell(float mx, float my, int* c, int* r) {
    int ww = 0, wh = 0;
    r3d_get_window_size(g_r3d, &ww, &wh);
    if (ww < 1) ww = 1;
    if (wh < 1) wh = 1;

    // 相机参数必须与 render_3d_frame 保持一致
    float target[3] = { 0.0f, 0.6f, 0.0f };
    float eye[3]; r3d_camera_around(eye, target);

    vec3_t E = { eye[0], eye[1], eye[2] };
    vec3_t T = { target[0], target[1], target[2] };
    vec3_t upWorld = { 0.0f, 1.0f, 0.0f };

    // 相机构建 basis（与 mat44_lookAt 的 f / right / up 一致）
    vec3_t fwd   = vec3_normalize(vec3_sub(T, E));
    vec3_t right = vec3_normalize(vec3_cross(fwd, upWorld));
    vec3_t up    = vec3_cross(right, fwd);

    float fovy = 55.0f;                                  // 与 render_3d_frame 的 55.0f 一致
    float aspect = (float)ww / (float)wh;
    float tanHalf = tanf(fovy * 3.14159265f / 360.0f);

    // 屏幕像素 -> NDC(-1..1), Y 向下翻转为向上
    float ndcX = (mx / (float)ww) * 2.0f - 1.0f;
    float ndcY = 1.0f - (my / (float)wh) * 2.0f;

    // 过该像素的射线方向 = fwd + right*(ndcX*tanHalf*aspect) + up*(ndcY*tanHalf)
    // 注意: SDL3 GPU 的 viewport/透视投影在竖直方向相对 NDC 是翻转的(与 2D HUD 的
    // proj.m5 取负是同一原因), 故这里对竖直分量取反, 使上下方向与屏幕一致。
    float dx = ndcX * tanHalf * aspect;
    float dy = -ndcY * tanHalf;
    vec3_t dir = {
        fwd.x + right.x * dx + up.x * dy,
        fwd.y + right.y * dx + up.y * dy,
        fwd.z + right.z * dx + up.z * dy,
    };
    dir = vec3_normalize(dir);

    // 射线与地平面 Y=0 相交(地面平面/floor 就画在 y=0)
    if (fabsf(dir.y) < 1e-5f) { *c = -1; *r = -1; return; }
    float t = -E.y / dir.y;
    if (t < 0.0f) { *c = -1; *r = -1; return; }   // 相机在地平面之下或视线背离地面(理论上不出现)

    float wx = E.x + dir.x * t;
    float wz = E.z + dir.z * t;
    map_world_to_cell(wx, wz, c, r);
}

// 当前工具颜色: 用于给 3D 里画"将要放置的块"预览用。
static void tool_color(int tool, jcolor_t* out) {
    switch (tool) {
    case 2: out->r=197; out->g=58; out->b=58; break;  // P 红
    case 3: out->r=200; out->g=137; out->b=42; break; // E 橙
    case 4: out->r=30; out->g=123; out->b=176; break; // X 蓝
    default: out->r=90; out->g=90; out->b=96; break;  // 墙灰
    }
    out->a = 255;
}

// 在 3D 窗口执行一次刷格动作(由鼠标事件触发)
static void r3d_paint(int c, int r, bool isLeft, bool isRight) {
    if (c < 0 || c >= MAP_EDIT_N || r < 0 || r >= MAP_EDIT_N) return;
    if (r == 0 || c == 0 || r == MAP_EDIT_N - 1 || c == MAP_EDIT_N - 1) {
        // 外圈强制墙, 不可改
        if (isLeft || isRight) g_map[r][c] = (char)TILE_WALL;
        return;
    }
    if (isLeft) {
        if (g_3dTool == 1) {
            // 若当前位置已有 P/X, 刷墙不覆盖标记(留给显式工具)。只落墙或空地做切换。
            char cur = g_map[r][c];
            if (cur == (char)TILE_P || cur == (char)TILE_X) { /* 保留标记 */ }
            else g_map[r][c] = (char)TILE_WALL;
        } else {
            // 放置标记 P/E/X。若该格已是墙, 先清为空地再放。
            if (g_map[r][c] == (char)TILE_WALL) g_map[r][c] = (char)TILE_EMPTY;
            char want = (char)TILE_E;
            if (g_3dTool == 2) want = (char)TILE_P;
            else if (g_3dTool == 3) want = (char)TILE_E;
            else if (g_3dTool == 4) want = (char)TILE_X;
            g_map[r][c] = want;
        }
    } else if (isRight) {
        char cur = g_map[r][c];
        if (cur == (char)TILE_WALL) g_map[r][c] = (char)TILE_EMPTY;
    }
}

// 渲染 3D 画刷窗口一帧(在 r3d_begin/r3d_end 之间调用)
static void render_3d_frame() {
    // 相机: 绕地图中心俯视
    float target[3] = { 0.0f, 0.6f, 0.0f };
    float eye[3]; r3d_camera_around(eye, target);
    vec3_t eyeV = { eye[0], eye[1], eye[2] };
    vec3_t tgtV = { target[0], target[1], target[2] };
    vec3_t upV  = { 0.0f, 1.0f, 0.0f };
    r3d_camera_lookat(g_r3d, eyeV, tgtV, upV, 55.0f);

    // 复用 mesh: 首次创建, 全程复用(避免每帧创建/销毁 GPU buffer 造成卡顿)
    if (!g_meshFloor) g_meshFloor = r3d_mesh_plane_lit(g_r3d, MAP_EDIT_N, MAP_EDIT_N, 1);
    if (!g_meshWall)  g_meshWall  = r3d_mesh_cube_lit(g_r3d, 1.0f, 1.0f, 1.0f);
    if (!g_meshMark)  g_meshMark  = r3d_mesh_cube_lit(g_r3d, 0.7f, 0.3f, 0.7f);
    if (!g_meshHover) g_meshHover = r3d_mesh_cube_wire(g_r3d, 1.0f, 1.0f, 1.0f);

    // 地面: 一块大平面_lit (20x20)
    mat44_t floorMat = mat44_identity();
    r3d_draw_mesh_lit_color(g_r3d, g_meshFloor, &floorMat, { 120, 130, 120, 255 });

    // 网格线
    r3d_draw_grid(g_r3d, MAP_EDIT_N, 1.0f, 1.0f);

    for (int r = 0; r < MAP_EDIT_N; r++) {
        for (int c = 0; c < MAP_EDIT_N; c++) {
            char ch = g_map[r][c];
            if (ch == (char)TILE_EMPTY) continue;
            float wx, wz; map_cell_to_world(c, r, &wx, &wz);
            if (ch == (char)TILE_WALL) {
                mat44_t m = mat44_translate((float)wx, 0.5f, (float)wz);
                r3d_draw_mesh_lit_color(g_r3d, g_meshWall, &m, { 96, 96, 104, 255 });
            } else {
                // 标记: 彩色扁块浮在格中心上方
                mat44_t m = mat44_translate((float)wx, 0.55f, (float)wz);
                jcolor_t cc;
                if (ch == (char)TILE_P) cc = { 197, 58, 58, 255 };
                else if (ch == (char)TILE_E) cc = { 200, 137, 42, 255 };
                else cc = { 30, 123, 176, 255 };
                r3d_draw_mesh_lit_color(g_r3d, g_meshMark, &m, cc);
            }
        }
    }

    // 悬停格高亮(线框)——用工具色描绘将要放置的块
    if (g_3dHoverC >= 0 && g_3dHoverR >= 0 && g_3dHoverC < MAP_EDIT_N && g_3dHoverR < MAP_EDIT_N) {
        float wx, wz; map_cell_to_world(g_3dHoverC, g_3dHoverR, &wx, &wz);
        jcolor_t hc; tool_color(g_3dTool, &hc);
        r3d_draw_wire(g_r3d, g_meshHover, { (float)wx, 0.5f, (float)wz }, { 1.0f, 1.0f, 1.0f }, hc);
    }

    // ---- 工具菜单 HUD: 2D 屏幕文字叠加在 3D 场景之上 ----
    // 用专用 quad 绘制, 颜色走 r3d_font_draw_2d_color / _outline(烘焙进纹理), 不重置为白色。
    int ww = 0, wh = 0;
    r3d_get_window_size(g_r3d, &ww, &wh);
    if (ww > 0 && wh > 0) {
        if (!g_meshText) g_meshText = r3d_mesh_quad(g_r3d);
        char line[160];
        const char* tn = g_3dTool==1 ? "刷墙" : (g_3dTool==2 ? "玩家P" : (g_3dTool==3 ? "敌人E" : "撤离X"));
        SDL_snprintf(line, sizeof(line), "工具: %s", tn);
        r3d_font_draw_2d_outline(g_r3d, g_meshText, line,
                                 12.0f, 12.0f, 22.0f, (float)ww, (float)wh,
                                 238, 238, 238, 18, 18, 18, 2.0f);
        SDL_snprintf(line, sizeof(line),
                     "1刷墙 2玩家P 3敌人E 4撤离X | Q/E旋转 R复位 F5导出 Esc关闭");
        r3d_font_draw_2d_outline(g_r3d, g_meshText, line,
                                 12.0f, 44.0f, 15.0f, (float)ww, (float)wh,
                                 205, 205, 205, 18, 18, 18, 1.5f);
    }
}

// ============================ SDL 回调 ============================

static game_timer_t g_timer;

// 字体路径: 与现有 2D 编辑器同目录(相对 build 工作目录)
static const char* k3dFont = "joy_editor/fonts/simhei.ttf";

static int  g_win3dId = 0;   // 3D 窗口的 SDL 窗口 ID(路由鼠标/键盘事件用)

// 创建并初始化 3D 画刷窗口(窗口B)。失败时返回 false 且不致命(可无 3D 继续用 2D)。
static bool r3d_init_window() {
    g_win3d = create_window("joy 地图画刷 (3D)", 900, 700);
    if (!g_win3d) { SDL_Log("r3d_init_window: create_window failed"); return false; }

    g_r3d = r3d_create(g_win3d, "joy_editor/shaders");
    if (!g_r3d) {
        SDL_Log("r3d_init_window: r3d_create failed");
        destroy_window(g_win3d); g_win3d = nullptr;
        return false;
    }

    // 缓存 3D 窗口 ID 供事件路由
    SDL_Window* raw3d = (SDL_Window*)get_raw_window(g_win3d);
    g_win3dId = SDL_GetWindowID(raw3d);

    if (!r3d_font_init(g_r3d, k3dFont, 24)) {
        SDL_Log("r3d_font_init failed(skip HUD text)");
        // 字体失败不致命, HUD 缺文字仍可刷墙
    }

    // 默认光照: 环境 + 方向光, 让墙盒有立体感
    jr3d_light_block_t lb; memset(&lb, 0, sizeof(lb));
    lb.ambient[0] = 0.35f; lb.ambient[1] = 0.35f; lb.ambient[2] = 0.38f; lb.ambient[3] = 1.0f;
    lb.lights[0].enabled = 1;
    lb.lights[0].type = JR3D_LIGHT_DIRECTIONAL;
    lb.lights[0].position[0] = -0.3f; lb.lights[0].position[1] = -1.0f; lb.lights[0].position[2] = 0.4f;
    lb.lights[0].color[0] = 1.0f; lb.lights[0].color[1] = 1.0f; lb.lights[0].color[2] = 1.0f; lb.lights[0].color[3] = 1.0f;
    r3d_set_light_block(g_r3d, &lb);

    g_3dActive = 1;
    con_log(0, "3D 地图画刷窗口已开启");
    return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    (void)appstate; (void)argc; (void)argv;

    if (!init_env()) {
        SDL_Log("init_env failed: %s", get_error());
        return SDL_APP_FAILURE;
    }

    g_win = create_window("joy editor", VIEW_W, VIEW_H);
    if (!g_win) { SDL_Log("create_window failed"); return SDL_APP_FAILURE; }

    g_r2d = r2d_create(g_win);
    if (!g_r2d) { SDL_Log("r2d_create failed"); return SDL_APP_FAILURE; }

    SDL_SetRenderLogicalPresentation(sdlr(), VIEW_W, VIEW_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderVSync(sdlr(), 1);

    g_font = font_create("joy_editor/fonts/simhei.ttf", 12);
    if (!g_font) {
        SDL_Log("font_create failed: joy_editor/fonts/simhei.ttf");
        return SDL_APP_FAILURE;
    }

    g_ui = jui_ctx_create(g_r2d, g_font);
    if (!g_ui) { SDL_Log("jui_ctx_create failed"); return SDL_APP_FAILURE; }

    game_timer_init(&g_timer);
    game_timer_set_target_fps(&g_timer, 60);

    fs_init();
    scene_reset();
    map_init_default();
    con_log(0, "joy 引擎编辑器已启动");
    con_log(0, "UI 层：jui_im（复用 joy 即时模式层）");
    con_log(0, "地图编辑：按 [F1] 打开/关闭 3D 画刷窗口");

    // 3D 画刷窗口: 默认打开(失败不阻塞 2D 编辑器)
    r3d_init_window();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* e) {
    (void)appstate;

    // 3D 画刷窗口事件: 鼠标拾取/刷格 + 相机旋转 + 工具/导出快捷键
    if (g_3dActive && e->type != SDL_EVENT_QUIT) {
        Uint32 ew = 0;
        switch (e->type) {
        case SDL_EVENT_MOUSE_MOTION: ew = e->motion.windowID; break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: case SDL_EVENT_MOUSE_BUTTON_UP: ew = e->button.windowID; break;
        case SDL_EVENT_MOUSE_WHEEL: ew = e->wheel.windowID; break;
        case SDL_EVENT_KEY_DOWN: case SDL_EVENT_KEY_UP: ew = e->key.windowID; break;
        default: break;
        }
        if (ew == (Uint32)g_win3dId) {
            // 鼠标运动: 更新悬停格
            if (e->type == SDL_EVENT_MOUSE_MOTION) {
                g_3dMouseX = e->motion.x; g_3dMouseY = e->motion.y;
                int c, r; r3d_pick_cell(g_3dMouseX, g_3dMouseY, &c, &r);
                g_3dHoverC = c; g_3dHoverR = r;
                if (g_3dPainting) r3d_paint(c, r, true, false);
            }
            else if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int c, r; r3d_pick_cell(e->button.x, e->button.y, &c, &r);
                g_3dHoverC = c; g_3dHoverR = r;
                bool left = (e->button.button == SDL_BUTTON_LEFT);
                bool right = (e->button.button == SDL_BUTTON_RIGHT);
                if (left || right) { g_3dPainting = true; r3d_paint(c, r, left, right); }
            }
            else if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
                g_3dPainting = false;
            }
            else if (e->type == SDL_EVENT_KEY_DOWN) {
                switch (e->key.scancode) {
                case SDL_SCANCODE_1: g_3dTool = 1; break;
                case SDL_SCANCODE_2: g_3dTool = 2; break;
                case SDL_SCANCODE_3: g_3dTool = 3; break;
                case SDL_SCANCODE_4: g_3dTool = 4; break;
                case SDL_SCANCODE_Q: g_3dCamYaw -= 15; break;
                case SDL_SCANCODE_E: g_3dCamYaw += 15; break;
                case SDL_SCANCODE_R: { g_3dCamPitch = -60; g_3dCamYaw = 0; g_3dCamDist = 18.0f; } break;
                case SDL_SCANCODE_F5: {
                    char buf[MAP_EDIT_N*(MAP_EDIT_N+1)+2];
                    map_export_ascii(buf, sizeof(buf));
                    // 写入项目内 ASCII 地图文件, 供游戏侧加载
                    const char* path = "joy_editor/level_edit.map";
                    SDL_IOStream* f = SDL_IOFromFile(path, "w");
                    if (f) {
                        SDL_WriteIO(f, buf, (size_t)strlen(buf));
                        SDL_CloseIO(f);
                        con_log(0, "地图已导出: %s", path);
                    } else {
                        con_log(1, "地图导出失败: %s", SDL_GetError());
                    }
                } break;
                case SDL_SCANCODE_ESCAPE: r3d_cleanup(); break;
                default: break;
                }
            }
            return SDL_APP_CONTINUE;  // 3D 窗口事件已消费
        }
    }

    switch (e->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_MOUSE_MOTION:
        jui_input_motion(g_ui, e->motion.x, e->motion.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        jui_input_motion(g_ui, e->button.x, e->button.y);
        if (e->button.button == SDL_BUTTON_LEFT) jui_input_down(g_ui, 0);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        jui_input_motion(g_ui, e->button.x, e->button.y);
        if (e->button.button == SDL_BUTTON_LEFT) jui_input_up(g_ui, 0);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        jui_input_wheel(g_ui, e->wheel.y);
        break;

    case SDL_EVENT_KEY_DOWN: {
        bool shift = (e->key.mod & SDL_KMOD_SHIFT) != 0;
        bool ctrl  = (e->key.mod & SDL_KMOD_CTRL)  != 0;
        jui_input_key(g_ui, shift, ctrl);
        if (e->key.scancode == SDL_SCANCODE_ESCAPE) { g_openMenu = -1; g_selected = -1; }
        if (e->key.scancode == SDL_SCANCODE_G) g_showGrid = !g_showGrid;
        if (e->key.scancode == SDL_SCANCODE_F1) {
            if (g_3dActive) r3d_cleanup();
            else r3d_init_window();
        }
        if (e->key.scancode == SDL_SCANCODE_DELETE && g_selected >= 0) {
            con_log(1, "已删除对象 #%d", g_selected);
            ent_remove(g_selected);
        }
        if (e->key.scancode == SDL_SCANCODE_F) {
            int i = ent_index(g_selected);
            if (i >= 0) { g_cam.x = g_ents[i].x; g_cam.y = g_ents[i].y; }
        }
        break;
    }
    case SDL_EVENT_KEY_UP: {
        bool shift = (e->key.mod & SDL_KMOD_SHIFT) != 0;
        bool ctrl  = (e->key.mod & SDL_KMOD_CTRL)  != 0;
        jui_input_key(g_ui, shift, ctrl);
        break;
    }

    default:
        break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    (void)appstate;

    game_timer_update(&g_timer);
    float dt = game_timer_get_delta_time(&g_timer);
    if (dt > 0.0001f) g_fps = g_fps * 0.9f + (1.0f / dt) * 0.1f;

    jui_begin(g_ui);
    jui_reset_clip(g_ui);

    r2d_set_color(g_r2d, 30, 30, 30, 255);
    r2d_begin(g_r2d);
    render_frame();
    jui_reset_clip(g_ui);
    r2d_end(g_r2d);

    // 定期回收闲置超过 600 帧的文字缓存条目
    static int sweepTick = 0;
    if (++sweepTick % 600 == 0) jtext_cache_sweep(600);

    jui_end(g_ui);

    // 状态栏跟随选中项
    static int lastSel = -2;
    if (lastSel != g_selected) {
        int i = ent_index(g_selected);
        if (i >= 0) {
            char b[128];
            SDL_snprintf(b, sizeof(b), "已选中 %s (#%d)",
                         g_ents[i].name.c_str(), g_ents[i].id);
            g_statusMsg = b;
        } else {
            g_statusMsg = "就绪";
        }
        lastSel = g_selected;
    }

    // 3D 画刷窗口渲染(独立 GPU 后端, 单独 begin/end, 不与 2D 冲突)
    if (g_3dActive && g_r3d) {
        if (r3d_begin(g_r3d)) {
            render_3d_frame();
            r3d_end(g_r3d);
        } else {
            // 窗口被关闭等导致 begin 失败: 做一次清理
            r3d_cleanup();
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    (void)appstate; (void)result;
    if (g_3dActive) r3d_cleanup();
    if (g_ui)   jui_ctx_destroy(g_ui);
    if (g_font) font_destroy(g_font);
    if (g_r2d)  r2d_destroy(g_r2d);
    if (g_win)  destroy_window(g_win);
}
