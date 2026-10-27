//#define SDL_MAIN_USE_CALLBACKS
//#include "SDL3/SDL.h"
//#include "SDL3/SDL_main.h"
//#include "SDL3/SDL_gpu.h"
//#include <joy/joy.h>
//#include <cmath>
//#include <cstring>
//#include <cstdio>
//#include <vector>
//#include <string>
//#include "joy/external/cgltf.h"   // 仅声明; 实现由 joy_editor 的 cgltf_impl.cpp 提供
//
///* gR3d 在文件下方定义, 此处前向声明供字体模块使用 */
//extern jr3d_p gR3d;
//extern void* gFloorTex;   // 诊断用(在文件下方定义)
//
///* ===================== 纯 r3d 字体渲染(不使用 r2d) ===================== */
//#define STB_TRUETYPE_IMPLEMENTATION
//#include "joy/external/stb_truetype.h"
//#undef STB_TRUETYPE_IMPLEMENTATION
//
//static stbtt_fontinfo gFontInfo;
//static SDL_GPUTexture* gFontTex = nullptr;   // 字体图集纹理(由 r3d 的 device 上传)
//static unsigned char*  gFontPixels = nullptr;
//static int             gFontAtlasW = 0, gFontAtlasH = 0;
//static int             gFontPixH = 48;       // 字形像素高度
//static int             gFontAscent = 0;      // 基线到字形顶部的偏移
//static struct { float x0, y0, x1, y1; int pxw, pxh; } gFontGlyphs[128]; // 每个字符在图集中的 UV 区域 + 像素尺寸
//static render_mesh_p gFontGlyphMeshes[128] = { nullptr };  // 每个字符的缓存 mesh(只创建一次, 不每帧释放)
//static float           gFontCellW = 0;       // 字形格子像素高
//static float           gFontScale = 0.0f;
//
//// 用 r3d 内部 device 上传 RGBA 像素为纹理(同 r3d_create_solid_texture 的格式)
//// 把一张纹理以 billboard 方式渲染到世界中(验证纹理路径 + 字体通用绘制)
//// 使用 jr3d 的 r3d_mesh_quad(索引 quad) + r3d_draw_mesh_tex_basis, 与示例验证路径一致
//static void draw_image_3d(void* tex, int texW, int texH, vec3_t center, float worldW, float yaw, float pitch)
//{
//    if (!tex || texW <= 0 || texH <= 0) return;
//    static render_mesh_p sQuad = nullptr;
//    if (!sQuad) sQuad = r3d_mesh_quad(gR3d);
//
//    float cy = cosf(yaw), sy = sinf(yaw);
//    float cp = cosf(pitch), sp = sinf(pitch);
//    vec3_t right = { -cy, 0.0f, sy };
//    vec3_t up    = { sp*sy, cp, sp*cy };
//    float ul = sqrtf(up.x*up.x + up.y*up.y + up.z*up.z);
//    if (ul > 1e-5f) { up.x /= ul; up.y /= ul; up.z /= ul; }
//
//    // 保持图片宽高比: 高度方向按 texH/texW 缩放 up
//    float aspect = (float)texH / (float)texW;
//    vec3_t upScaled = { up.x * aspect, up.y * aspect, up.z * aspect };
//
//    // 单位 quad 经 right/up 基向量旋转 + worldW 缩放 + center 平移
//    r3d_draw_mesh_tex_basis(gR3d, sQuad, center, right, upScaled, worldW, tex);
//}
//
//// 初始化字体: 加载 ttf + 把 ASCII 32..126 烘焙进单行图集(参考 2D 文本渲染的 baseline 对齐)
//static void font_init(const char* ttf_path)
//{
//    static unsigned char ttf_buf[1 << 20];
//    size_t n = 0;
//    void* data = SDL_LoadFile(ttf_path, &n);
//    if (!data || n == 0 || n > sizeof(ttf_buf)) { SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "load font %s failed", ttf_path); return; }
//    memcpy(ttf_buf, data, n); SDL_free(data);
//    stbtt_InitFont(&gFontInfo, ttf_buf, 0);
//    gFontScale = stbtt_ScaleForPixelHeight(&gFontInfo, (float)gFontPixH);
//
//    int ascent = 0, descent = 0, lineGap = 0;
//    stbtt_GetFontVMetrics(&gFontInfo, &ascent, &descent, &lineGap);
//    gFontAscent = (int)(ascent * gFontScale);
//
//    int cellH = gFontAscent + (int)(descent * gFontScale) + 2; // 字形格子高度(含基线)
//    if (cellH < gFontPixH) cellH = gFontPixH + 4;
//
//    // 第一遍: 模拟排布, 算出需要多少行, 避免 UV 越界/换行错位
//    gFontAtlasW = 1024;
//    int simPenX = 1, rows = 1;
//    for (int c = 32; c < 127; c++) {
//        int bx0, by0, bx1, by1;
//        stbtt_GetCodepointBitmapBox(&gFontInfo, c, gFontScale, gFontScale, &bx0, &by0, &bx1, &by1);
//        int bw = bx1 - bx0, bh = by1 - by0;
//        int maxGlyph = gFontPixH * 3;
//        int cellW = (bw > 0 && bw <= maxGlyph) ? (bw + 3) : ((int)((gFontPixH * 0.6f)) + 3);
//        if (simPenX + cellW > gFontAtlasW - 2) { rows++; simPenX = 1; }
//        simPenX += cellW;
//    }
//    gFontAtlasH = rows * (cellH + 4) + 4;
//    gFontPixels = (unsigned char*)SDL_calloc((size_t)gFontAtlasW * gFontAtlasH, 4);
//    if (!gFontPixels) return;
//
//    int penX = 1, penY = 1;
//    int lineH = cellH + 4;
//    for (int c = 32; c < 127; c++) {
//        int adv = 0, lsb = 0;
//        stbtt_GetCodepointHMetrics(&gFontInfo, c, &adv, &lsb);
//        int bx0, by0, bx1, by1;
//        stbtt_GetCodepointBitmapBox(&gFontInfo, c, gFontScale, gFontScale, &bx0, &by0, &bx1, &by1);
//        int bw = bx1 - bx0, bh = by1 - by0;
//        int maxGlyph = gFontPixH * 3; // 合理上限, 避免异常 bbox 导致 malloc 爆内存
//        if (bw <= 0 || bh <= 0 || bw > maxGlyph || bh > maxGlyph) { // 空格/无轮廓/异常: 跳过
//            gFontGlyphs[c].x0 = gFontGlyphs[c].y0 = gFontGlyphs[c].x1 = gFontGlyphs[c].y1 = 0;
//            penX += (int)(adv * gFontScale) + 1;
//            continue;
//        }
//        if (penX + bw > gFontAtlasW - 2) { penX = 1; penY += lineH; }
//        // 先把字形写进独立 buffer(避免 by0 负偏移导致越界写入图集)
//        size_t tmpSize = (size_t)bw * (size_t)bh;
//        unsigned char* tmp = (unsigned char*)SDL_malloc(tmpSize > 0 ? tmpSize : 1);
//        if (!tmp) { penX += bw + 3; continue; }
//        stbtt_MakeCodepointBitmap(&gFontInfo, tmp, bw, bh, bw, gFontScale, gFontScale, c);
//        // 复制到图集: 字形顶部对齐格子顶部(penY), 保证不越界
//        int dstTop = penY;
//        if (dstTop + bh <= gFontAtlasH) {
//            for (int yy = 0; yy < bh; yy++) {
//                for (int xx = 0; xx < bw; xx++) {
//                    unsigned char a = tmp[yy * bw + xx];
//                    int idx = ((dstTop + yy) * gFontAtlasW + (penX + xx)) * 4;
//                    gFontPixels[idx + 0] = 255;
//                    gFontPixels[idx + 1] = 255;
//                    gFontPixels[idx + 2] = 255;
//                    gFontPixels[idx + 3] = a;
//                }
//            }
//        }
//        SDL_free(tmp);
//        // UV 用字形实际像素范围(带行偏移 penY), 确保 UV 严格 < 1.0, 避免 REPEAT 平铺
//        float u0 = ((float)penX + 1.0f) / gFontAtlasW;
//        float u1 = ((float)(penX + bw) - 1.0f) / gFontAtlasW;
//        float v0 = ((float)penY + 1.0f) / gFontAtlasH;
//        float v1 = ((float)(penY + bh) - 1.0f) / gFontAtlasH;
//        if (u0 < 0) u0 = 0; if (u1 > 1.0f) u1 = 1.0f;
//        if (v0 < 0) v0 = 0; if (v1 > 1.0f) v1 = 1.0f;
//        gFontGlyphs[c].x0 = u0; gFontGlyphs[c].y0 = v0;
//        gFontGlyphs[c].x1 = u1; gFontGlyphs[c].y1 = v1;
//        // 记录像素尺寸, 供 draw 时按字形宽高比设置 quad
//        gFontGlyphs[c].pxw = bw; gFontGlyphs[c].pxh = bh;
//        penX += bw + 3;
//    }
//    gFontCellW = (float)cellH; // 记下格子像素高
//    gFontTex = (SDL_GPUTexture*)r3d_create_texture(gR3d, gFontAtlasW, gFontAtlasH, gFontPixels, 0);
//
//    // 调试: 把图集导出为 BMP, 供检查字形是否正常
//    {
//        FILE* fp = fopen("joy_editor/font_atlas_debug.bmp", "wb");
//        if (fp) {
//            int w = gFontAtlasW, h = gFontAtlasH;
//            int rowBytes = (w * 3 + 3) & ~3;
//            int pad = rowBytes - w * 3;
//            int dataSize = rowBytes * h;
//            unsigned char hdr[54];
//            memset(hdr, 0, sizeof(hdr));
//            hdr[0]='B'; hdr[1]='M';
//            unsigned int filesz = 54 + dataSize;
//            memcpy(hdr+2,&filesz,4);
//            hdr[10]=54;
//            hdr[14]=40; hdr[18]=w&0xff; hdr[19]=(w>>8)&0xff; hdr[20]=(w>>16)&0xff; hdr[21]=(w>>24)&0xff;
//            hdr[22]=h&0xff; hdr[23]=(h>>8)&0xff; hdr[24]=(h>>16)&0xff; hdr[25]=(h>>24)&0xff;
//            hdr[26]=1; hdr[28]=24;
//            unsigned int imsz = dataSize;
//            memcpy(hdr+34,&imsz,4);
//            fwrite(hdr,1,54,fp);
//            for (int y = h-1; y >= 0; y--) { // BMP 自底向上
//                for (int x = 0; x < w; x++) {
//                    int idx = (y*w + x)*4;
//                    unsigned char r=gFontPixels[idx+0], gg=gFontPixels[idx+1], b=gFontPixels[idx+2], a=gFontPixels[idx+3];
//                    // 在黑色背景上用白色字形(按 alpha 混合)便于查看
//                    fputc((unsigned char)(b*a/255), fp);
//                    fputc((unsigned char)(gg*a/255), fp);
//                    fputc((unsigned char)(r*a/255), fp);
//                }
//                for (int i=0;i<pad;i++) fputc(0,fp);
//            }
//            fclose(fp);
//        }
//    }
//    SDL_Log("FONT DEBUG: atlas=%dx%d cellH=%d glyphs: ", gFontAtlasW, gFontAtlasH, cellH);
//    for (int cc = 'A'; cc <= 'Z'; cc++)
//        SDL_Log("  [%c] pxw=%d pxh=%d u0=%.3f u1=%.3f v0=%.3f v1=%.3f",
//            cc, gFontGlyphs[cc].pxw, gFontGlyphs[cc].pxh,
//            gFontGlyphs[cc].x0, gFontGlyphs[cc].x1,
//            gFontGlyphs[cc].y0, gFontGlyphs[cc].y1);
//}
//
//// 在世界空间绘制一行文字(billboard 面向相机)
//// center: 文字中心位置; size: 字符高度; yaw: 相机水平角; pitch: 相机俯仰角
//static void draw_text_3d(const char* str, vec3_t center, float size, float yaw, float pitch)
//{
//    if (!gFontTex) return;
//    // 相机坐标系轴: 右向量 & 上向量(billboard 朝向相机)
//    // 相机 fwd = {sin(yaw)*cp, -sp, cos(yaw)*cp}
//    // right = normalize(cross(fwd, worldUp)), up = cross(right, fwd)
//    float cy = cosf(yaw), sy = sinf(yaw);
//    float cp = cosf(pitch), sp = sinf(pitch);
//    // 相机观察方向 fwd = (sy*cp, -sp, cy*cp)
//    // right = normalize(cross(fwd, worldUp)) = (-cy, 0, sy)   (已单位化)
//    // up    = normalize(cross(right, fwd))  = (sp*sy, cp, sp*cy) (需单位化, 注意 z 用 cy)
//    vec3_t right = { -cy, 0.0f, sy };                 // 相机水平右(文字阅读方向, 已单位化)
//    vec3_t up    = { sp*sy, cp, sp*cy };              // 相机屏幕向上(未单位化, 下面归一化)
//    {
//        float ul = sqrtf(up.x*up.x + up.y*up.y + up.z*up.z);
//        if (ul > 1e-5f) { up.x /= ul; up.y /= ul; up.z /= ul; }
//    }
//    float halfH = size * 0.5f;
//    // 计算整行总宽, 让文字居中
//    float totalW = 0;
//    for (const char* p = str; *p; p++) {
//        int c = (unsigned char)*p;
//        totalW += (c >= 32 && c <= 126) ? size * ((float)gFontGlyphs[c].pxw / (float)gFontGlyphs[c].pxh) + size * 0.15f : size * 0.5f;
//    }
//    float xOff = -totalW * 0.5f; // 起始横向偏移(相对中心)
//    for (const char* p = str; *p; p++) {
//        int c = (unsigned char)*p;
//        if (c < 32 || c > 126) { xOff += size * 0.5f; continue; }
//        auto& g = gFontGlyphs[c];
//        if (g.x1 - g.x0 > 0.0001f) {
//            float wdt = size * ((float)g.pxw / (float)g.pxh);
//            // billboard 顶点: 中心 + 沿 right/up 偏移
//            vec3_t c0 = { center.x + right.x*xOff + up.x*halfH,
//                          center.y + right.y*xOff + up.y*halfH,
//                          center.z + right.z*xOff + up.z*halfH };
//            vec3_t c1 = { center.x + right.x*(xOff+wdt) + up.x*halfH,
//                          center.y + right.y*(xOff+wdt) + up.y*halfH,
//                          center.z + right.z*(xOff+wdt) + up.z*halfH };
//            vec3_t c2 = { center.x + right.x*(xOff+wdt) - up.x*halfH,
//                          center.y + right.y*(xOff+wdt) - up.y*halfH,
//                          center.z + right.z*(xOff+wdt) - up.z*halfH };
//            vec3_t c3 = { center.x + right.x*xOff - up.x*halfH,
//                          center.y + right.y*xOff - up.y*halfH,
//                          center.z + right.z*xOff - up.z*halfH };
//            float v[6 * 5] = {
//                c0.x, c0.y, c0.z,  g.x0, g.y0,
//                c1.x, c1.y, c1.z,  g.x1, g.y0,
//                c2.x, c2.y, c2.z,  g.x1, g.y1,
//                c0.x, c0.y, c0.z,  g.x0, g.y0,
//                c2.x, c2.y, c2.z,  g.x1, g.y1,
//                c3.x, c3.y, c3.z,  g.x0, g.y1,
//            };
//            // 每个字符只创建一次 mesh, 之后复用(避免每帧 CreateBuffer+Release 触发崩溃)
//            render_mesh_p m = gFontGlyphMeshes[c];
//            if (!m) {
//                m = r3d_mesh_custom(gR3d, v, 6);
//                gFontGlyphMeshes[c] = m;
//            }
//            if (m) {
//                vec3_t posZero = {0, 0, 0};
//                vec3_t sclOne = {1, 1, 1};
//                r3d_draw_mesh_tex(gR3d, m, posZero, sclOne, gFontTex);
//            }
//            xOff += wdt + size * 0.15f;
//        } else {
//            xOff += size * 0.5f;
//        }
//    }
//}
//
//static vec3f_t toV3f(vec3_t v) { return { fp_from_float(v.x), fp_from_float(v.y), fp_from_float(v.z) }; }
//
//// -------------------- GPU 资源 (jr3d) --------------------
//jwindow_p gWin = nullptr;
//jr3d_p gR3d = nullptr;
//
//jr3d_light_block_t gLightBlock;
//render_mesh_p gFloorMesh = nullptr;
//render_mesh_p gPlayerMesh = nullptr;
//render_mesh_p gUnitCubeMesh = nullptr;
//render_mesh_p gSphereMesh = nullptr;
//render_mesh_p gBulletMesh = nullptr;
//render_mesh_p gRampMesh = nullptr;
//void* gFloorTex = nullptr;
//void* gLogoTex = nullptr;       // logo.png 纹理(用于验证纯 r3d 纹理渲染路径)
//int   gLogoTexW = 0, gLogoTexH = 0;
//
//#define WINDOW_WIDTH 480
//#define WINDOW_HEIGHT 320
//
//// 玩家
//vec3_t gPlayerPos = {0.0f, 0.6f, 0.0f};
//int gFrameCount = 0;
//float gPlayerYaw = 0.0f;       // 玩家模型朝向(跟随移动方向)
//float gCameraYaw = 0.0f;       // 相机水平朝向(鼠标拖动旋转)
//float gPitch = 60.0f;          // 相机俯角(60 度俯视角, MOBA 风格)
//static const float CAM_DIST = 8.0f;
//
//// 时间
//uint64_t gLastTime = 0;
//float gDeltaTime = 0.0f;
//
//// 输入
//struct {
//    bool forward = false;
//    bool backward = false;
//    bool left = false;
//    bool right = false;
//    bool jump = false;
//    bool shoot = false;
//} gKeys;
//
//float gVerticalVelocity = 0.0f;
//bool gOnGround = true;
//
//struct DrawMesh {
//    render_mesh_p mesh;
//    void* tex;         // 纹理，NULL 时使用 color
//    jcolor_t color;
//};
//
//std::vector<DrawMesh> gDrawMeshes;
//std::vector<mat44_t> gModelMatrices;
//
//struct BoxObstacle {
//    vec3_t pos;
//    vec3_t size;
//    gjk3df_collider_t gjkShape;
//    BoxObstacle() { memset(&gjkShape, 0, sizeof(gjkShape)); }
//};
//
//std::vector<BoxObstacle> gBoxes;
//
//// GJK 测试球体
//struct BallObstacle {
//    vec3_t pos;
//    float radius;
//    gjk3df_collider_t gjkShape;
//    BallObstacle() { memset(&gjkShape, 0, sizeof(gjkShape)); }
//};
//std::vector<BallObstacle> gBalls;
//
//// 旋转箱子（OBB，GJK 支持但 AABB 不支持）
//struct OBBObstacle {
//    vec3_t pos;
//    vec3_t size;
//    float angle;
//    gjk3df_collider_t gjkShape;
//    vec3f_t worldVerts[8];
//};
//std::vector<OBBObstacle> gOBBs;
//
//// 斜坡（长方体斜放作为斜坡; triPrism=true 时为三角棱柱, 角点在 basePos, 与渲染 mesh 顶点一致）
//struct RampObstacle {
//    vec3_t basePos;
//    float width, length, height;
//    float angle;  // 绕X轴旋转角度（斜放长方体时使用）
//    bool triPrism = false;
//    gjk3df_collider_t gjkShape;
//    vec3f_t worldVerts[8];
//
//    float getHeight(float x, float z) const {
//        if (triPrism) {
//            // 三角棱柱: 角点在 basePos, 沿 +Z 从 y=0 升到 y=height, 坡面 y = (z-bz)/length*height
//            float zl = z - basePos.z;
//            if (zl < 0.0f) zl = 0.0f;
//            if (zl > length) zl = length;
//            return basePos.y + zl / length * height;
//        }
//        float cs = cosf(angle), sn = sinf(angle);
//        float hh = height * 0.5f, hl = length * 0.5f;
//        // 局部Z坐标
//        float z_local = (z - basePos.z - hh * sn) / cs;
//        if (z_local < -hl) z_local = -hl;
//        if (z_local > hl) z_local = hl;
//        return basePos.y + hh * cs - z_local * sn + hh;
//    }
//
//    bool isInXZ(float x, float z) const {
//        if (triPrism) {
//            return x >= basePos.x - 0.2f && x <= basePos.x + width + 0.2f &&
//                   z >= basePos.z - 0.2f && z <= basePos.z + length + 0.2f;
//        }
//        float cs = cosf(angle), sn = sinf(angle);
//        float hw = width * 0.5f, hl = length * 0.5f, hh = height * 0.5f;
//        float x_local = x - basePos.x;
//        float z_local = (z - basePos.z - hh * sn) / cs;
//        return fabsf(x_local) <= hw + 0.2f && fabsf(z_local) <= hl + 0.2f;
//    }
//};
//
//std::vector<RampObstacle> gRamps;
//
//// 敌人
//struct Enemy {
//    vec3_t pos;
//    float hp = 3.0f;
//    float speed = 2.0f;
//    float aggroRange = 8.0f;      // 发现玩家距离
//    float attackRange = 1.2f;     // 攻击距离
//    float attackCooldown = 1.0f;
//    float attackTimer = 0.0f;
//    int damage = 1;
//    bool alive = true;
//};
//
//std::vector<Enemy> gEnemies;
//int gPlayerHP = 10;
//int gPlayerMaxHP = 10;
//
//// 子弹
//struct Bullet {
//    vec3_t pos;
//    vec3_t dir;
//    float speed = 15.0f;
//    float lifetime = 2.0f;
//    float age = 0.0f;
//    int damage = 1;
//    bool active = true;
//};
//
//std::vector<Bullet> gBullets;
//float gShootCooldown = 0.25f;
//float gShootTimer = 0.0f;
//
//static const vec3_t PLAYER_HALF_SIZE = {0.3f, 0.6f, 0.3f};
//
//// ====================================================================
////  骨骼动画角色 (glTF skin + animation, CPU 蒙皮)
//// ====================================================================
//struct SkinnedVertex {
//    vec3_t pos;            // 原始局部坐标
//    vec3_t nrm;
//    vec2_t uv;
//    unsigned int joints[4];
//    float weights[4];
//};
//
//struct SkinnedModel {
//    bool loaded = false;
//    int vertCount = 0;
//    std::vector<SkinnedVertex> base;     // 原始(未蒙皮)顶点
//    std::vector<float> skinnedBuf;       // 每帧输出: 8 float/顶点 (pos+normal+uv)
//    cgltf_data* data = nullptr;
//    cgltf_skin* skin = nullptr;
//    cgltf_animation* anim = nullptr;
//    int jointCount = 0;                  // = skin->joints_count
//    std::vector<mat44_t> jointLocal;     // 每 node 当前 local 变换
//    std::vector<mat44_t> jointGlobal;
//    std::vector<mat44_t> jointMatrix;    // global * inverseBind, 按 skin->joints 顺序
//    std::vector<mat44_t> inverseBind;
//    float animTime = 0.0f;
//    float animDuration = 1.0f;
//    // 归一化: 把模型缩放到约 PLAYER_HALF_SIZE.y*2 高, 脚底对齐 y=0
//    float modelScale = 1.0f;
//    // 预烘焙关键帧(整段动画一次性烘焙为静态 mesh, 运行时只切换指针, 零每帧 buffer 创建/释放)
//    std::vector<render_mesh_p> bakedMeshes;
//    int bakedCount = 0;
//    // 脚底原始最小 Y(归一化后整体下移, 让脚底贴 y=0)
//    float baseMinY = 0.0f;
//};
//
//static SkinnedModel gPlayerSkin;
//
//// cgltf 的 float[16] (列主序) -> mat44_t
//static mat44_t cgltf_to_mat44(const float m[16]) {
//    mat44_t r;
//    r.m0  = m[0];  r.m4  = m[4];  r.m8  = m[8];  r.m12 = m[12];
//    r.m1  = m[1];  r.m5  = m[5];  r.m9  = m[9];  r.m13 = m[13];
//    r.m2  = m[2];  r.m6  = m[6];  r.m10 = m[10]; r.m14 = m[14];
//    r.m3  = m[3];  r.m7  = m[7];  r.m11 = m[11]; r.m15 = m[15];
//    return r;
//}
//
//// 四元数(xyzw) -> mat44_t (列主序)
//static mat44_t quat_to_mat44(float x, float y, float z, float w) {
//    mat44_t r;
//    float xx = x*x, yy = y*y, zz = z*z;
//    float xy = x*y, xz = x*z, yz = y*z;
//    float wx = w*x, wy = w*y, wz = w*z;
//    r.m0  = 1 - 2*(yy + zz);  r.m4  = 2*(xy - wz);      r.m8  = 2*(xz + wy);      r.m12 = 0;
//    r.m1  = 2*(xy + wz);      r.m5  = 1 - 2*(xx + zz);  r.m9  = 2*(yz - wx);      r.m13 = 0;
//    r.m2  = 2*(xz - wy);      r.m6  = 2*(yz + wx);      r.m10 = 1 - 2*(xx + yy);  r.m14 = 0;
//    r.m3  = 0;                r.m7  = 0;                r.m11 = 0;                r.m15 = 1;
//    return r;
//}
//
//static mat44_t mat_translate(float x, float y, float z) {
//    mat44_t r = mat44_identity();
//    r.m12 = x; r.m13 = y; r.m14 = z;
//    return r;
//}
//static mat44_t mat_scale(float x, float y, float z) {
//    mat44_t r = mat44_identity();
//    r.m0 = x; r.m5 = y; r.m10 = z;
//    return r;
//}
//
//// 取 node 的静态 T/R/S, 合成 local (动画会覆盖其中分量)
//static void initNodeLocal(SkinnedModel& m, int idx) {
//    cgltf_node* node = &m.data->nodes[idx];
//    vec3_t t = {0,0,0};
//    if (node->has_translation) { t.x = node->translation[0]; t.y = node->translation[1]; t.z = node->translation[2]; }
//    mat44_t R = mat44_identity();
//    if (node->has_rotation) R = quat_to_mat44(node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]);
//    vec3_t s = {1,1,1};
//    if (node->has_scale) { s.x = node->scale[0]; s.y = node->scale[1]; s.z = node->scale[2]; }
//    mat44_t T = mat_translate(t.x, t.y, t.z);
//    mat44_t S = mat_scale(s.x, s.y, s.z);
//    m.jointLocal[idx] = mat44_mul(mat44_mul(T, R), S);
//}
//
//// 对单个 channel 在 animTime 处采样, 覆盖对应 node 的 local 分量
//static void sampleChannel(SkinnedModel& m, const cgltf_animation_channel& ch) {
//    cgltf_animation_sampler& samp = *ch.sampler;
//    cgltf_node* node = ch.target_node;
//    int idx = (int)(node - m.data->nodes);
//
//    cgltf_accessor* in = samp.input;
//    cgltf_accessor* out = samp.output;
//    float t = m.animTime;
//    int count = (int)in->count;
//    // 找区间
//    int i0 = 0;
//    for (int i = 0; i < count - 1; i++) {
//        float a = 0, b = 0;
//        cgltf_accessor_read_float(in, i, &a, 1);
//        cgltf_accessor_read_float(in, i+1, &b, 1);
//        if (t >= a && t <= b) { i0 = i; break; }
//        i0 = i;
//    }
//    float t0 = 0, t1 = 0;
//    cgltf_accessor_read_float(in, i0, &t0, 1);
//    cgltf_accessor_read_float(in, (i0+1 < count) ? i0+1 : i0, &t1, 1);
//    float f = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;
//    if (f < 0) f = 0; if (f > 1) f = 1;
//
//    int comps = (int)out->count > 0 ? 0 : 0;
//    (void)comps;
//    int outComps = 0;
//    switch (ch.target_path) {
//        case cgltf_animation_path_type_translation: outComps = 3; break;
//        case cgltf_animation_path_type_scale:        outComps = 3; break;
//        case cgltf_animation_path_type_rotation:     outComps = 4; break;
//        default: outComps = 0; break;
//    }
//    if (outComps == 0) return;
//
//    std::vector<float> v0(outComps), v1(outComps);
//    cgltf_accessor_read_float(out, i0, v0.data(), outComps);
//    cgltf_accessor_read_float(out, (i0+1 < count) ? i0+1 : i0, v1.data(), outComps);
//
//    // 重新从当前 local 分解 T/R/S 并覆盖对应分量
//    cgltf_node* nd = &m.data->nodes[idx];
//    vec3_t tpos = {0,0,0};
//    if (nd->has_translation) { tpos.x = nd->translation[0]; tpos.y = nd->translation[1]; tpos.z = nd->translation[2]; }
//    float quat[4] = {0,0,0,1};
//    if (nd->has_rotation) { quat[0]=nd->rotation[0]; quat[1]=nd->rotation[1]; quat[2]=nd->rotation[2]; quat[3]=nd->rotation[3]; }
//    vec3_t scl = {1,1,1};
//    if (nd->has_scale) { scl.x = nd->scale[0]; scl.y = nd->scale[1]; scl.z = nd->scale[2]; }
//
//    if (ch.target_path == cgltf_animation_path_type_translation) {
//        float* tp = &tpos.x;
//        for (int k = 0; k < 3; k++) tp[k] = v0[k] + (v1[k] - v0[k]) * f;
//    } else if (ch.target_path == cgltf_animation_path_type_scale) {
//        float* sc = &scl.x;
//        for (int k = 0; k < 3; k++) sc[k] = v0[k] + (v1[k] - v0[k]) * f;
//    } else if (ch.target_path == cgltf_animation_path_type_rotation) {
//        // 对四元数做 nlerp (足够 demo 用)
//        float dot = v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] + v0[3]*v1[3];
//        float s = (dot < 0) ? -1.0f : 1.0f;
//        for (int k = 0; k < 4; k++) quat[k] = v0[k] + (v1[k]*s - v0[k]) * f;
//        float len = sqrtf(quat[0]*quat[0] + quat[1]*quat[1] + quat[2]*quat[2] + quat[3]*quat[3]);
//        if (len > 1e-6f) { quat[0]/=len; quat[1]/=len; quat[2]/=len; quat[3]/=len; }
//    }
//
//    mat44_t T = mat_translate(tpos.x, tpos.y, tpos.z);
//    mat44_t R = quat_to_mat44(quat[0], quat[1], quat[2], quat[3]);
//    mat44_t S = mat_scale(scl.x, scl.y, scl.z);
//    m.jointLocal[idx] = mat44_mul(mat44_mul(T, R), S);
//}
//
//// 递归 FK
//static void updateNodeGlobal(SkinnedModel& m, cgltf_node* node) {
//    int idx = (int)(node - m.data->nodes);
//    mat44_t local = m.jointLocal[idx];
//    mat44_t global;
//    if (node->parent) {
//        int pidx = (int)(node->parent - m.data->nodes);
//        global = mat44_mul(m.jointGlobal[pidx], local);
//    } else {
//        global = local;
//    }
//    m.jointGlobal[idx] = global;
//    for (int i = 0; i < (int)node->children_count; i++)
//        updateNodeGlobal(m, node->children[i]);
//}
//
//static void loadSkinnedModel(SkinnedModel& m, const char* path) {
//    cgltf_options opts;
//    memset(&opts, 0, sizeof(opts));
//    cgltf_data* data = nullptr;
//    cgltf_result res = cgltf_parse_file(&opts, path, &data);
//    if (res != cgltf_result_success || !data) {
//        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "loadSkinnedModel: 无法解析 %s (err=%d), 回退到立方体", path, (int)res);
//        return;
//    }
//    cgltf_load_buffers(&opts, data, path);
//
//    // 找第一个带 JOINTS_0 的 primitive
//    cgltf_mesh* mesh = nullptr;
//    cgltf_primitive* prim = nullptr;
//    for (int i = 0; i < (int)data->meshes_count; i++) {
//        for (int j = 0; j < (int)data->meshes[i].primitives_count; j++) {
//            cgltf_primitive* p = &data->meshes[i].primitives[j];
//            bool hasJ = false, hasW = false;
//            for (int a = 0; a < (int)p->attributes_count; a++) {
//                if (p->attributes[a].type == cgltf_attribute_type_joints) hasJ = true;
//                if (p->attributes[a].type == cgltf_attribute_type_weights) hasW = true;
//            }
//            if (hasJ && hasW) { mesh = &data->meshes[i]; prim = p; break; }
//        }
//        if (prim) break;
//    }
//    // 即使没有 skin 属性, 也退而求其次用第一个 primitive
//    if (!prim && data->meshes_count > 0) {
//        mesh = &data->meshes[0];
//        prim = &data->meshes[0].primitives[0];
//    }
//    if (!prim) { cgltf_free(data); return; }
//
//    cgltf_accessor* aPos = nullptr, *aNrm = nullptr, *aUv = nullptr, *aJ = nullptr, *aW = nullptr;
//    for (int a = 0; a < (int)prim->attributes_count; a++) {
//        cgltf_attribute& at = prim->attributes[a];
//        if (at.type == cgltf_attribute_type_position) aPos = at.data;
//        else if (at.type == cgltf_attribute_type_normal) aNrm = at.data;
//        else if (at.type == cgltf_attribute_type_texcoord && at.index == 0) aUv = at.data;
//        else if (at.type == cgltf_attribute_type_joints) aJ = at.data;
//        else if (at.type == cgltf_attribute_type_weights) aW = at.data;
//    }
//
//    int vcount = aPos ? (int)aPos->count : 0;
//    if (vcount == 0) { cgltf_free(data); return; }
//
//    // 解析索引(若有)
//    std::vector<uint32_t> indices;
//    int outVerts = 0;
//    if (prim->indices) {
//        int idxCount = (int)prim->indices->count;
//        indices.resize(idxCount);
//        cgltf_accessor_unpack_indices(prim->indices, indices.data(), sizeof(uint32_t), idxCount);
//        outVerts = idxCount;
//    } else {
//        outVerts = vcount;
//    }
//
//    m.base.resize(outVerts);
//    for (int i = 0; i < outVerts; i++) {
//        int vi = prim->indices ? (int)indices[i] : i;
//        SkinnedVertex& v = m.base[i];
//        float p[3] = {0,0,0};
//        if (aPos) cgltf_accessor_read_float(aPos, vi, p, 3);
//        v.pos = {p[0], p[1], p[2]};
//        float n[3] = {0,1,0};
//        if (aNrm) cgltf_accessor_read_float(aNrm, vi, n, 3);
//        v.nrm = {n[0], n[1], n[2]};
//        float uv[2] = {0,0};
//        if (aUv) cgltf_accessor_read_float(aUv, vi, uv, 2);
//        v.uv = {uv[0], uv[1]};
//        for (int k = 0; k < 4; k++) { v.joints[k] = 0; v.weights[k] = (k==0)?1.0f:0.0f; }
//        if (aJ) {
//            int ji[4] = {0,0,0,0};
//            cgltf_accessor_read_float(aJ, vi, (float*)ji, 4);  // JOINTS_0 是 uint, 但读成 float 再取整
//            for (int k = 0; k < 4; k++) v.joints[k] = (unsigned int)ji[k];
//        }
//        if (aW) {
//            float w[4] = {0,0,0,0};
//            cgltf_accessor_read_float(aW, vi, w, 4);
//            for (int k = 0; k < 4; k++) v.weights[k] = w[k];
//        }
//    }
//    m.vertCount = outVerts;
//    m.skinnedBuf.resize((size_t)outVerts * 8);
//
//    // 包围盒 -> 归一化缩放/偏移 (脚底对齐 y=0, 高度 ≈ 1.2)
//    float minY = 1e9f, maxY = -1e9f, maxAbsXZ = 1e-3f;
//    for (int i = 0; i < outVerts; i++) {
//        auto& p = m.base[i].pos;
//        if (p.y < minY) minY = p.y;
//        if (p.y > maxY) maxY = p.y;
//        float axz = sqrtf(p.x*p.x + p.z*p.z);
//        if (axz > maxAbsXZ) maxAbsXZ = axz;
//    }
//    float height = (maxY - minY);
//    if (height < 1e-3f) height = 1.0f;
//    m.modelScale = (PLAYER_HALF_SIZE.y * 2.0f) / height;
//    m.baseMinY = minY;  // 记录原始最小 Y, 蒙皮时减去让脚底对齐 y=0
//
//    m.data = data;
//    m.skin = (data->skins_count > 0) ? &data->skins[0] : nullptr;
//    m.anim = (data->animations_count > 0) ? &data->animations[0] : nullptr;
//
//    int nodeCount = (int)data->nodes_count;
//    m.jointLocal.resize(nodeCount);
//    m.jointGlobal.resize(nodeCount);
//    for (int i = 0; i < nodeCount; i++) initNodeLocal(m, i);
//
//    if (m.skin) {
//        m.jointCount = (int)m.skin->joints_count;
//        m.inverseBind.resize(m.jointCount);
//        if (m.skin->inverse_bind_matrices) {
//            for (int i = 0; i < m.jointCount; i++) {
//                float ibm[16];
//                cgltf_accessor_read_float(m.skin->inverse_bind_matrices, i, ibm, 16);
//                m.inverseBind[i] = cgltf_to_mat44(ibm);
//            }
//        } else {
//            for (int i = 0; i < m.jointCount; i++) m.inverseBind[i] = mat44_identity();
//        }
//        m.jointMatrix.resize(m.jointCount);
//    }
//
//    if (m.anim) {
//        // 取最大时间作为输入时长
//        float maxT = 0.0f;
//        for (int i = 0; i < (int)m.anim->samplers_count; i++) {
//            cgltf_accessor* in = m.anim->samplers[i].input;
//            float last = 0;
//            if (in->count > 0) cgltf_accessor_read_float(in, (int)in->count - 1, &last, 1);
//            if (last > maxT) maxT = last;
//        }
//        m.animDuration = (maxT > 0) ? maxT : 1.0f;
//    }
//
//    m.loaded = true;
//    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "loadSkinnedModel: %s 顶点=%d 关节=%d 动画时长=%.2fs mesh='%s' primitives=%d anims=%d",
//                path, outVerts, m.jointCount, m.animDuration,
//                mesh && mesh->name ? mesh->name : "(no name)",
//                mesh ? (int)mesh->primitives_count : 0,
//                data ? (int)data->animations_count : 0);
//}
//
//// 每帧更新骨骼 + CPU 蒙皮, 返回可用于 r3d_mesh_custom_lit 的 8float 缓冲
//static const float* updateSkinnedModel(SkinnedModel& m, float dt) {
//    if (!m.loaded) return nullptr;
//    if (m.anim) {
//        m.animTime += dt;
//        if (m.animTime > m.animDuration) m.animTime -= m.animDuration;
//        // 先用静态 T/R/S 重置 local, 再让动画 channel 覆盖
//        int nodeCount = (int)m.data->nodes_count;
//        for (int i = 0; i < nodeCount; i++) initNodeLocal(m, i);
//        for (int i = 0; i < (int)m.anim->channels_count; i++)
//            sampleChannel(m, m.anim->channels[i]);
//    }
//    // FK
//    if (m.data->nodes_count > 0)
//        updateNodeGlobal(m, &m.data->nodes[0]);
//
//    // 计算 jointMatrix (skin->joints 顺序)
//    if (m.skin) {
//        for (int i = 0; i < m.jointCount; i++) {
//            int nidx = (int)(m.skin->joints[i] - m.data->nodes);
//            m.jointMatrix[i] = mat44_mul(m.jointGlobal[nidx], m.inverseBind[i]);
//        }
//    }
//
//    // 蒙皮: 输出 8 float/顶点 (pos+normal+uv)
//    float* out = m.skinnedBuf.data();
//    for (int i = 0; i < m.vertCount; i++) {
//        SkinnedVertex& v = m.base[i];
//        vec3_t sp = {0,0,0}, sn = {0,0,0};
//        for (int k = 0; k < 4; k++) {
//            float w = v.weights[k];
//            if (w <= 0.0f) continue;
//            mat44_t M = mat44_identity();
//            if (m.skin && v.joints[k] < (unsigned int)m.jointCount) M = m.jointMatrix[v.joints[k]];
//            else M = mat44_identity();
//            // pos (w=1)
//            float x = v.pos.x, y = v.pos.y, z = v.pos.z;
//            sp.x += w * (M.m0*x + M.m4*y + M.m8*z + M.m12);
//            sp.y += w * (M.m1*x + M.m5*y + M.m9*z + M.m13);
//            sp.z += w * (M.m2*x + M.m6*y + M.m10*z + M.m14);
//            // normal (w=0, 忽略平移)
//            float nx = v.nrm.x, ny = v.nrm.y, nz = v.nrm.z;
//            sn.x += w * (M.m0*nx + M.m4*ny + M.m8*nz);
//            sn.y += w * (M.m1*nx + M.m5*ny + M.m9*nz);
//            sn.z += w * (M.m2*nx + M.m6*ny + M.m10*nz);
//        }
//        // 归一化缩放 + 脚底对齐(减去 baseMinY 让脚底贴 y=0)
//        int o = i * 8;
//        out[o+0] = sp.x * m.modelScale;
//        out[o+1] = (sp.y - m.baseMinY) * m.modelScale;
//        out[o+2] = sp.z * m.modelScale;
//        float nl = sqrtf(sn.x*sn.x + sn.y*sn.y + sn.z*sn.z);
//        if (nl > 1e-6f) { sn.x/=nl; sn.y/=nl; sn.z/=nl; }
//        out[o+3] = sn.x;
//        out[o+4] = sn.y;
//        out[o+5] = sn.z;
//        out[o+6] = v.uv.x;
//        out[o+7] = v.uv.y;
//    }
//    return out;
//}
//
//// 预烘焙整段动画为静态 mesh 序列(不再每帧创建/释放 GPU buffer, 根除 DEVICE_LOST)
//static void bakeSkinnedFrames(SkinnedModel& m, jr3d_p r3d, int fps) {
//    if (!m.loaded || fps <= 0) return;
//    for (auto* mesh : m.bakedMeshes) { if (mesh) r3d_mesh_destroy(r3d, mesh); }
//    m.bakedMeshes.clear();
//
//    int totalFrames = (int)(m.animDuration * fps);
//    if (totalFrames < 1) totalFrames = 1;
//    float frameDt = 1.0f / (float)fps;
//
//    float savedTime = m.animTime;
//    for (int f = 0; f < totalFrames; f++) {
//        m.animTime = (float)f * frameDt;
//        const float* buf = updateSkinnedModel(m, 0.0f);  // 内部按 animTime 重算 pose(无 GPU 操作)
//        render_mesh_p mesh = r3d_mesh_custom_lit(r3d, buf, m.vertCount);
//        m.bakedMeshes.push_back(mesh);
//    }
//    m.animTime = savedTime;
//    m.bakedCount = (int)m.bakedMeshes.size();
//    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BAKE frames=%d", m.bakedCount);
//}
//
//// 仅用于诊断: 返回首帧蒙皮缓冲
//static const float* m_skin_first_frame(SkinnedModel& m) {
//    float saved = m.animTime;
//    m.animTime = 0.0f;
//    const float* buf = updateSkinnedModel(m, 0.0f);
//    m.animTime = saved;
//    return buf;
//}
//
//// -------------------- 初始化 --------------------
//bool initSDL() {
//    gWin = create_window("Skinned Character Demo", WINDOW_WIDTH, WINDOW_HEIGHT);
//    if (!gWin) {
//        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "create_window failed: %s", SDL_GetError());
//        return false;
//    }
//    gR3d = r3d_create(gWin, "joy_editor/shaders");
//    if (!gR3d) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "r3d_create failed: %s", SDL_GetError());
//        return false;
//    }
//    gLastTime = get_ticks();
//    return true;
//}
//
//// -------------------- 创建网格 (jr3d) --------------------
//void createMeshes() {
//    gFloorMesh = r3d_mesh_plane_lit(gR3d, 20.0f, 20.0f, 10);
//    gFloorTex = r3d_gen_checkerboard(gR3d, 32, 10);
//    gUnitCubeMesh = r3d_mesh_cube_lit(gR3d, 1.0f, 1.0f, 1.0f);
//    gSphereMesh = r3d_mesh_sphere_lit(gR3d, 0.5f, 16, 12);
//    gBulletMesh = r3d_mesh_cube_lit(gR3d, 0.16f, 0.16f, 0.16f);
//    gRampMesh = r3d_mesh_ramp_lit(gR3d, 2.0f, 3.0f, 1.5f);
//
//    // 字体: 用纯 r3d 接口渲染(不使用 r2d)
//    font_init("joy_editor/fonts/arial.ttf");
//
//    // 测试: 先渲染一张图片(logo.png) 验证纹理路径, 再处理字体
//    // 正确路径固定为 joy_editor/textures/logo.png(工作目录为 build/<config>)
//    gLogoTex = r3d_load_texture_png(gR3d, "joy_editor/textures/logo.png", &gLogoTexW, &gLogoTexH);
//
//    // 玩家: 尝试加载骨骼动画角色 robot.glb (位于 assets/joy_editor/models/)
//    // CMake 会复制到 build/<config>/joy_editor/joy_editor/models/robot.glb, 工作目录不同需多试几个相对路径
//    const char* candidates[] = {
//        "joy_editor/models/robot.glb",
//        "models/robot.glb",
//        "../joy_editor/models/robot.glb",
//        "../../joy_editor/joy_editor/models/robot.glb"
//    };
//    for (int ci = 0; ci < 4; ci++) {
//        loadSkinnedModel(gPlayerSkin, candidates[ci]);
//        if (gPlayerSkin.loaded) break;
//    }
//    if (gPlayerSkin.loaded) {
//        // 预烘焙整段动画为静态 mesh 序列(运行时只切换指针, 零每帧 buffer 创建/释放)
//        bakeSkinnedFrames(gPlayerSkin, gR3d, 24);
//        // 诊断：定位"模型不显示"
//        const float* buf = m_skin_first_frame(gPlayerSkin);
//        bool nan = false;
//        for (int i = 0; i < gPlayerSkin.vertCount * 8; i++) if (!(buf[i] == buf[i])) { nan = true; break; }
//        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
//            "PLAYER MESH vertCount=%d baked=%d firstPos=(%.3f,%.3f,%.3f) hasNaN=%d",
//            gPlayerSkin.vertCount, gPlayerSkin.bakedCount,
//            gPlayerSkin.vertCount > 0 ? buf[0] : 0.0f,
//            gPlayerSkin.vertCount > 0 ? buf[1] : 0.0f,
//            gPlayerSkin.vertCount > 0 ? buf[2] : 0.0f,
//            (int)nan);
//    } else {
//        // 回退到立方体
//        gPlayerMesh = r3d_mesh_cube_lit(gR3d, 0.6f, 1.2f, 0.6f);
//    }
//}
//
//// 检测玩家在 pos 处是否与场景碰撞（curPos 为当前实际位置, 用于"站在物体上"的跳过判断）
//// 箱子用 AABB 精确分离测试: 恰好贴合(穿透<=EPS)不算碰撞, 保证贴墙时可沿墙滑动
//static bool collideAt(const vec3_t& pos, const vec3_t& curPos) {
//    const float EPS = 0.02f;  // 接触容差
//    gjk3df_collider_t playerShape;
//    gjk3df_init_box(&playerShape, toV3f(pos), toV3f(PLAYER_HALF_SIZE));
//    vec3f_t gjkInitDir = { fp_one(), fp_zero(), fp_zero() };
//
//    // 箱子（AABB）
//    for (auto& box : gBoxes) {
//        float boxTop = box.pos.y + box.size.y * 0.5f;
//        // 玩家站在箱子顶部时跳过碰撞（仅当脚底几乎贴住箱顶表面）
//        float playerBottom = pos.y - PLAYER_HALF_SIZE.y;
//        bool onBox = playerBottom >= boxTop - 0.01f && playerBottom <= boxTop + 0.05f &&
//            fabsf(pos.x - box.pos.x) <= box.size.x * 0.5f + PLAYER_HALF_SIZE.x * 0.5f &&
//            fabsf(pos.z - box.pos.z) <= box.size.z * 0.5f + PLAYER_HALF_SIZE.z * 0.5f;
//        // 往边缘走时 pos 可能略微超出, 但当前位置还在箱子上
//        if (!onBox) {
//            float curBottom = curPos.y - PLAYER_HALF_SIZE.y;
//            onBox = curBottom >= boxTop - 0.01f && curBottom <= boxTop + 0.05f &&
//                fabsf(curPos.x - box.pos.x) <= box.size.x * 0.5f + PLAYER_HALF_SIZE.x * 0.5f &&
//                fabsf(curPos.z - box.pos.z) <= box.size.z * 0.5f + PLAYER_HALF_SIZE.z * 0.5f;
//        }
//        if (onBox) continue;
//        vec3_t bh = { box.size.x * 0.5f, box.size.y * 0.5f, box.size.z * 0.5f };
//        if (fabsf(pos.x - box.pos.x) < PLAYER_HALF_SIZE.x + bh.x - EPS &&
//            fabsf(pos.y - box.pos.y) < PLAYER_HALF_SIZE.y + bh.y - EPS &&
//            fabsf(pos.z - box.pos.z) < PLAYER_HALF_SIZE.z + bh.z - EPS)
//            return true;
//    }
//
//    // 球 / OBB（GJK + 接触容差）
//    for (auto& ball : gBalls) {
//        gjk3df_contact_t c;
//        if (gjk3df_collide(&playerShape, &ball.gjkShape, gjkInitDir, &c) && c.depth > fp_from_float(EPS)) return true;
//    }
//    for (auto& obb : gOBBs) {
//        gjk3df_contact_t c;
//        if (gjk3df_collide(&playerShape, &obb.gjkShape, gjkInitDir, &c) && c.depth > fp_from_float(EPS)) return true;
//    }
//
//    // 斜坡（玩家站在坡上时跳过碰撞）
//    for (auto& ramp : gRamps) {
//        bool onRamp = ramp.isInXZ(pos.x, pos.z) &&
//            fabsf(pos.y - PLAYER_HALF_SIZE.y - ramp.getHeight(pos.x, pos.z)) < 0.3f;
//        // 从斜坡顶往边缘走时, isInXZ(pos) 会 false, 但当前位置还在坡上
//        if (!onRamp) {
//            onRamp = ramp.isInXZ(curPos.x, curPos.z) &&
//                fabsf(curPos.y - PLAYER_HALF_SIZE.y - ramp.getHeight(curPos.x, curPos.z)) < 0.3f;
//        }
//        if (onRamp) continue;
//        gjk3df_contact_t c;
//        if (gjk3df_collide(&playerShape, &ramp.gjkShape, gjkInitDir, &c) && c.depth > fp_from_float(EPS)) return true;
//    }
//    return false;
//}
//
//// -------------------- 输入处理 --------------------
//void processInput() {
//    const bool* kb = SDL_GetKeyboardState(NULL);
//    gKeys.forward  = kb[SDL_SCANCODE_W];
//    gKeys.backward = kb[SDL_SCANCODE_S];
//    gKeys.left     = kb[SDL_SCANCODE_A];
//    gKeys.right    = kb[SDL_SCANCODE_D];
//    gKeys.jump     = kb[SDL_SCANCODE_SPACE];
//    gKeys.shoot    = kb[SDL_SCANCODE_F];
//
//    // MOBA 视角：WASD 移动方向相对于屏幕（相机朝向），与角色朝向无关
//    float camYawRad = ft_radians(gCameraYaw);
//    vec3_t forward = {sinf(camYawRad), 0, cosf(camYawRad)};
//    vec3_t right   = {cosf(camYawRad), 0, -sinf(camYawRad)};
//
//    vec3_t moveDir = {0,0,0};
//    if (gKeys.forward)  moveDir = vec3_add(moveDir, forward);
//    if (gKeys.backward) moveDir = vec3_sub(moveDir, forward);
//    if (gKeys.left)     moveDir = vec3_add(moveDir, right);
//    if (gKeys.right)    moveDir = vec3_sub(moveDir, right);
//
//    if (vec3_length(moveDir) > 0.001f) {
//        moveDir = vec3_normalize(moveDir);
//        // 角色朝向跟随移动方向
//        gPlayerYaw = atan2f(moveDir.x, moveDir.z) * 180.0f / 3.14159265f;
//
//        float totalDist = 5.0f * gDeltaTime;
//        const float MAX_STEP = 0.02f;
//        while (totalDist > 0.0001f) {
//            float step = (totalDist > MAX_STEP) ? MAX_STEP : totalDist;
//            totalDist -= step;
//
//            vec3_t newPos = vec3_add(gPlayerPos, vec3_scale(moveDir, step));
//
//            // 爬坡：新位置在斜坡 XZ 范围内时自动贴合坡面
//            for (auto& ramp : gRamps) {
//                if (ramp.isInXZ(newPos.x, newPos.z)) {
//                    float rampY = ramp.getHeight(newPos.x, newPos.z);
//                    float playerBottom = newPos.y - PLAYER_HALF_SIZE.y;
//                    if (playerBottom <= rampY + 0.2f) {
//                        newPos.y = rampY + PLAYER_HALF_SIZE.y + 0.01f;
//                    }
//                    break;
//                }
//            }
//
//            // 碰撞检测 + 滑动解析: 完整移动被挡时, 分别保留 X/Z 轴向分量, 实现贴墙滑动
//            if (!collideAt(newPos, gPlayerPos)) {
//                gPlayerPos = newPos;
//            } else {
//                bool moved = false;
//                vec3_t tryX = { newPos.x, newPos.y, gPlayerPos.z };
//                if (!collideAt(tryX, gPlayerPos)) { gPlayerPos = tryX; moved = true; }
//                if (!moved) {
//                    vec3_t tryZ = { gPlayerPos.x, newPos.y, newPos.z };
//                    if (!collideAt(tryZ, gPlayerPos)) { gPlayerPos = tryZ; moved = true; }
//                }
//                if (!moved) break;  // X/Z 两方向都顶死, 停止移动
//            }
//        }
//    }
//
//    // 空气墙：限制在地板边界内（地板范围 ±10）
//    if (gPlayerPos.x > 9.5f) gPlayerPos.x = 9.5f;
//    if (gPlayerPos.x < -9.5f) gPlayerPos.x = -9.5f;
//    if (gPlayerPos.z > 9.5f) gPlayerPos.z = 9.5f;
//    if (gPlayerPos.z < -9.5f) gPlayerPos.z = -9.5f;
//
//    // 计算地面高度（斜坡、箱子顶部、地面中取最高值）
//    float groundY = 0.0f;  // 默认地面 y = 0
//
//    // 斜坡（高度 + GJK 侧向碰撞已在移动检测中处理）
//    for (auto& ramp : gRamps) {
//        if (ramp.isInXZ(gPlayerPos.x, gPlayerPos.z)) {
//            float sy = ramp.getHeight(gPlayerPos.x, gPlayerPos.z);
//            if (sy > groundY) groundY = sy;
//        }
//    }
//    // 箱子顶部
//    for (auto& box : gBoxes) {
//        float boxTop = box.pos.y + box.size.y * 0.5f;
//        float boxBottom = box.pos.y - box.size.y * 0.5f;
//        // 使用 GJK 做 XZ 平面重叠检测
//        gjk3df_collider_t playerFeetShape;
//        gjk3df_init_box(&playerFeetShape, toV3f({gPlayerPos.x, boxTop, gPlayerPos.z}), toV3f({PLAYER_HALF_SIZE.x * 0.8f, 0.01f, PLAYER_HALF_SIZE.z * 0.8f}));
//        vec3f_t gjkInitDir = { fp_one(), fp_zero(), fp_zero() };
//        gjk3df_contact_t c;
//        if (gjk3df_collide(&playerFeetShape, &box.gjkShape, gjkInitDir, &c)) {
//            float playerFeet = gPlayerPos.y - PLAYER_HALF_SIZE.y;
//            float playerHead = gPlayerPos.y + PLAYER_HALF_SIZE.y;
//
//            // 玩家脚底在箱顶附近（从上方落下）→ 站在箱顶
//            if (playerFeet >= boxTop - 0.3f && playerFeet <= boxTop + 0.1f) {
//                if (boxTop > groundY) groundY = boxTop;
//            }
//            // 玩家头顶在箱底附近（从下方跳起）→ 撞头，止住上升
//            if (playerHead >= boxBottom - 0.1f && playerHead <= boxBottom + 0.3f && gVerticalVelocity > 0) {
//                gPlayerPos.y = boxBottom - PLAYER_HALF_SIZE.y;
//                gVerticalVelocity = 0.0f;
//            }
//        }
//    }
//
//    // 跳跃/落地
//    float playerFeetY = gPlayerPos.y - PLAYER_HALF_SIZE.y;
//    gOnGround = (playerFeetY <= groundY + 0.01f);
//
//    if (gKeys.jump && gOnGround) {
//        gVerticalVelocity = 4.0f;
//        gOnGround = false;
//    }
//
//    // 重力
//    if (!gOnGround) {
//        gVerticalVelocity -= 9.8f * gDeltaTime;
//    }
//
//    // 应用垂直速度
//    gPlayerPos.y += gVerticalVelocity * gDeltaTime;
//
//    // 落地检测
//    playerFeetY = gPlayerPos.y - PLAYER_HALF_SIZE.y;
//    if (playerFeetY < groundY) {
//        gPlayerPos.y = groundY + PLAYER_HALF_SIZE.y;
//        gVerticalVelocity = 0.0f;
//        gOnGround = true;
//    }
//}
//
//// -------------------- 更新 MVP --------------------
//void updateMVPData() {
//    // 地板
//    gModelMatrices[0] = mat44_identity();
//
//    // 玩家（骨骼动画角色）: 推进动画时间并选择预烘焙关键帧 mesh(零每帧 buffer 创建/释放)
//    if (gPlayerSkin.loaded && gPlayerSkin.bakedCount > 0) {
//        // 仅推进动画时间(CPU), 不触碰任何 GPU 资源
//        if (gPlayerSkin.anim) {
//            gPlayerSkin.animTime += gDeltaTime;
//            if (gPlayerSkin.animTime > gPlayerSkin.animDuration)
//                gPlayerSkin.animTime -= gPlayerSkin.animDuration;
//        }
//        int idx = (int)(gPlayerSkin.animTime * 24.0f) % gPlayerSkin.bakedCount;
//        if (idx < 0) idx = 0;
//        gPlayerMesh = gPlayerSkin.bakedMeshes[idx];
//        gDrawMeshes[1].mesh = gPlayerMesh;
//    }
//    {
//        mat44_t m = mat44_identity();
//        float y = ft_radians(gPlayerYaw), c = cosf(y), s = sinf(y);
//        // 注意: glTF 模型可能正面朝 -Z, 这里加 180° 让角色正面朝移动方向(+Z)
//        m.m0 = -c;  m.m2  = s;
//        m.m8 = -s;  m.m10 = -c;
//        m.m12 = gPlayerPos.x;
//        m.m13 = gPlayerPos.y;
//        m.m14 = gPlayerPos.z;
//        gModelMatrices[1] = m;
//    }
//
//    // 旧斜坡（三角棱柱，index 2）
//    gModelMatrices[2] = mat44_translate(gRamps[0].basePos.x, gRamps[0].basePos.y, gRamps[0].basePos.z);
//
//    // 新斜坡（斜放长方体，index 3）
//    {
//        mat44_t m = mat44_identity();
//        auto& ramp = gRamps[1];
//        float cs = cosf(ramp.angle), sn = sinf(ramp.angle);
//        m.m0  = ramp.width;
//        m.m5  = ramp.height * cs;  m.m6  = ramp.height * sn;
//        m.m9  = ramp.length * -sn; m.m10 = ramp.length * cs;
//        m.m12 = ramp.basePos.x;
//        m.m13 = ramp.basePos.y + ramp.height * 0.5f;
//        m.m14 = ramp.basePos.z;
//        gModelMatrices[3] = m;
//    }
//
//    // 障碍物箱子（斜坡 index 2/3，箱子从 4 开始）
//    for (size_t i = 0; i < gBoxes.size(); i++) {
//        auto& box = gBoxes[i];
//        mat44_t m = mat44_identity();
//        m.m0  = box.size.x;
//        m.m5  = box.size.y;
//        m.m10 = box.size.z;
//        m.m12 = box.pos.x;
//        m.m13 = box.pos.y;
//        m.m14 = box.pos.z;
//        gModelMatrices[4 + i] = m;
//    }
//}
//
//// -------------------- 主循环 --------------------
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//    if (!initSDL()) return SDL_APP_FAILURE;
//
//    createMeshes();
//
//    // 初始化环境光 + 太阳光
//    memset(&gLightBlock, 0, sizeof(gLightBlock));
//    gLightBlock.ambient[0] = 0.15f; gLightBlock.ambient[1] = 0.15f; gLightBlock.ambient[2] = 0.15f; gLightBlock.ambient[3] = 1.0f;
//    gLightBlock.lights[0].enabled = 1;
//    gLightBlock.lights[0].type = JR3D_LIGHT_DIRECTIONAL;
//    gLightBlock.lights[0].position[0] = 30.0f; gLightBlock.lights[0].position[1] = 25.0f; gLightBlock.lights[0].position[2] = 30.0f;
//    gLightBlock.lights[0].target[0] = 0.0f; gLightBlock.lights[0].target[1] = -1.0f; gLightBlock.lights[0].target[2] = 0.0f;
//    gLightBlock.lights[0].color[0] = 1.0f; gLightBlock.lights[0].color[1] = 0.95f; gLightBlock.lights[0].color[2] = 0.8f; gLightBlock.lights[0].color[3] = 1.0f;
//
//    // 地板
//    gModelMatrices.push_back(mat44_identity());
//    gDrawMeshes.push_back({ gFloorMesh, gFloorTex, {0, 0, 0} });
//
//    // 玩家（骨骼动画角色; 用颜色绘制, 与原来 cube 一致）
//    gModelMatrices.push_back(mat44_identity());
//    gDrawMeshes.push_back({ gPlayerMesh, nullptr, {60, 140, 255} });
//
//    gRamps.reserve(8);  // 预分配,避免 push_back 时重分配导致 gjkShape.vertices 指针悬空
//
//    // 旧斜坡（三角棱柱）
//    {
//        gRamps.emplace_back();
//        auto& ramp = gRamps.back();
//        ramp.basePos = {-6, 0, 0};
//        ramp.width = 2.0f;
//        ramp.length = 3.0f;
//        ramp.height = 1.5f;
//        ramp.angle = 0;
//        ramp.triPrism = true;
//        {
//            // 顶点与 r3d_mesh_ramp_lit(w,l,h) 的 A B C D E F 一一对应（basePos 为角点）
//            ramp.worldVerts[0] = toV3f({ramp.basePos.x,                ramp.basePos.y,                ramp.basePos.z});
//            ramp.worldVerts[1] = toV3f({ramp.basePos.x + ramp.width,   ramp.basePos.y,                ramp.basePos.z});
//            ramp.worldVerts[2] = toV3f({ramp.basePos.x,                ramp.basePos.y + ramp.height,  ramp.basePos.z + ramp.length});
//            ramp.worldVerts[3] = toV3f({ramp.basePos.x + ramp.width,   ramp.basePos.y + ramp.height,  ramp.basePos.z + ramp.length});
//            ramp.worldVerts[4] = toV3f({ramp.basePos.x,                ramp.basePos.y,                ramp.basePos.z + ramp.length});
//            ramp.worldVerts[5] = toV3f({ramp.basePos.x + ramp.width,   ramp.basePos.y,                ramp.basePos.z + ramp.length});
//            gjk3df_init_mesh(&ramp.gjkShape, ramp.worldVerts, 6);
//        }
//        mat44_t m = mat44_translate(ramp.basePos.x, ramp.basePos.y, ramp.basePos.z);
//        gModelMatrices.push_back(m);
//        gDrawMeshes.push_back({ gRampMesh, nullptr, {180, 140, 90} });
//    }
//
//    // 新斜坡（长方体斜放，放地图另一侧）
//    {
//        gRamps.emplace_back();
//        auto& ramp = gRamps.back();
//        ramp.basePos = {6, 0, 0};
//        ramp.width = 2.0f;
//        ramp.length = 3.0f;
//        ramp.height = 1.0f;
//        ramp.angle = -atan2f(ramp.height, ramp.length);  // 与三角斜坡同向（+Z 方向升高）
//        {
//            float cs = cosf(ramp.angle), sn = sinf(ramp.angle);
//            float hw = ramp.width * 0.5f, hh = ramp.height * 0.5f, hl = ramp.length * 0.5f;
//            float cx = ramp.basePos.x, cy = ramp.basePos.y + hh, cz = ramp.basePos.z;
//            int vi = 0;
//            for (int ix = -1; ix <= 1; ix += 2)
//                for (int iy = -1; iy <= 1; iy += 2)
//                    for (int iz = -1; iz <= 1; iz += 2) {
//                        float lx = ix * hw, ly = iy * hh, lz = iz * hl;
//                        float wx = lx + cx, wy = ly * cs - lz * sn + cy, wz = ly * sn + lz * cs + cz;
//                        ramp.worldVerts[vi++] = toV3f({wx, wy, wz});
//                    }
//            gjk3df_init_mesh(&ramp.gjkShape, ramp.worldVerts, 8);
//        }
//        {
//            mat44_t m = mat44_identity();
//            float cs = cosf(ramp.angle), sn = sinf(ramp.angle);
//            m.m0  = ramp.width;
//            m.m5  = ramp.height * cs;  m.m6  = ramp.height * sn;
//            m.m9  = ramp.length * -sn; m.m10 = ramp.length * cs;
//            m.m12 = ramp.basePos.x;
//            m.m13 = ramp.basePos.y + ramp.height * 0.5f;
//            m.m14 = ramp.basePos.z;
//            gModelMatrices.push_back(m);
//            gDrawMeshes.push_back({ gUnitCubeMesh, nullptr, {180, 140, 90} });
//        }
//    }
//
//    // 创建障碍物箱子
//    {
//        struct { float x, z; float sx, sy, sz; } boxDefs[] = {
//            { 3, 3, 1, 1, 1 },
//            { -3, -2, 1, 0.5f, 1 },
//            { 5, -4, 0.8f, 1.2f, 0.8f },
//            { -4, 4, 1, 1, 1 },
//            { -5, -4, 1.5f, 0.8f, 1.5f },
//            { 4, -5, 0.5f, 1.5f, 0.5f },
//            { 0, 5, 1, 1, 1.5f },
//            { -1, -5, 1.2f, 0.6f, 1.2f },
//        };
//        for (auto& def : boxDefs) {
//            BoxObstacle box;
//            box.pos = {def.x, def.sy * 0.5f, def.z};
//            box.size = {def.sx, def.sy, def.sz};
//            gjk3df_init_box(&box.gjkShape, toV3f(box.pos), toV3f({box.size.x * 0.5f, box.size.y * 0.5f, box.size.z * 0.5f}));
//            gBoxes.push_back(box);
//
//            mat44_t m = mat44_identity();
//            m.m0  = box.size.x;
//            m.m5  = box.size.y;
//            m.m10 = box.size.z;
//            m.m12 = box.pos.x;
//            m.m13 = box.pos.y;
//            m.m14 = box.pos.z;
//            gModelMatrices.push_back(m);
//            gDrawMeshes.push_back({ gUnitCubeMesh, nullptr, {150, 110, 70} });
//        }
//    }
//
//    // 创建 GJK 测试球体
//    {
//        BallObstacle ball;
//        ball.pos = {2, 0.5f, 2};
//        ball.radius = 0.5f;
//        gjk3df_init_sphere(&ball.gjkShape, toV3f(ball.pos), fp_from_float(ball.radius));
//        gBalls.push_back(ball);
//        {
//            mat44_t m = mat44_identity();
//            m.m0 = ball.radius * 2; m.m5 = ball.radius * 2; m.m10 = ball.radius * 2;
//            m.m12 = ball.pos.x; m.m13 = ball.pos.y; m.m14 = ball.pos.z;
//            gModelMatrices.push_back(m);
//            gDrawMeshes.push_back({ gSphereMesh, nullptr, {60, 200, 90} });
//        }
//    }
//
//    // 创建 GJK 测试 OBB（旋转 45° 的箱子，AABB 无法精确处理）
//    {
//        gOBBs.emplace_back();
//        auto& obb = gOBBs.back();
//        obb.pos = {-2, 0.5f, 2};
//        obb.size = {0.8f, 0.8f, 0.8f};
//        obb.angle = 45.0f;
//        {
//            vec3f_t half = toV3f({obb.size.x * 0.5f, obb.size.y * 0.5f, obb.size.z * 0.5f});
//            vec3f_t localVerts[8] = {
//                { -half.x,  half.y, -half.z }, {  half.x,  half.y, -half.z },
//                {  half.x,  half.y,  half.z }, { -half.x,  half.y,  half.z },
//                { -half.x, -half.y, -half.z }, {  half.x, -half.y, -half.z },
//                {  half.x, -half.y,  half.z }, { -half.x, -half.y,  half.z },
//            };
//            float rad = ft_radians(obb.angle);
//            mat44f_t t = mat44f_translate(fp_from_float(obb.pos.x), fp_from_float(obb.pos.y), fp_from_float(obb.pos.z));
//            mat44f_t r = mat44f_rotate_y(fp_from_float(rad));
//            // 先绕原点旋转，再平移到 obb.pos：worldVerts = T·R·v
//            mat44f_t transform = mat44f_mul(t, r);
//            for (int i = 0; i < 8; i++) {
//                vec4f_t v = { localVerts[i].x, localVerts[i].y, localVerts[i].z, fp_one() };
//                vec4f_t res = mat44f_mul_vec4f(transform, v);
//                obb.worldVerts[i] = { res.x, res.y, res.z };
//            }
//            gjk3df_init_mesh(&obb.gjkShape, obb.worldVerts, 8);
//        }
//        {
//            float y = ft_radians(obb.angle), c = cosf(y), s = sinf(y);
//            mat44_t m = mat44_identity();
//            m.m0 = obb.size.x * c;  m.m2  = obb.size.x * -s;
//            m.m5 = obb.size.y;
//            m.m8 = obb.size.z * s;  m.m10 = obb.size.z * c;
//            m.m12 = obb.pos.x; m.m13 = obb.pos.y; m.m14 = obb.pos.z;
//            gModelMatrices.push_back(m);
//            gDrawMeshes.push_back({ gUnitCubeMesh, nullptr, {255, 165, 0} });
//        }
//    }
//
//    // 太阳圆盘：在远处天空放一个大方块
//    {
//        mat44_t m = mat44_identity();
//        m.m0 = 3.0f; m.m5 = 3.0f; m.m10 = 3.0f;
//        m.m12 = 30.0f; m.m13 = 25.0f; m.m14 = 30.0f;  // 远处高空
//        gModelMatrices.push_back(m);
//        gDrawMeshes.push_back({ gUnitCubeMesh, nullptr, {255, 240, 200} });
//    }
//
//#if 0  // 敌人已屏蔽
//    {
//        struct { float x, z; } enemyPos[] = {
//            { 4, 4 }, { -4, -3 }, { 6, -5 }, { -5, 5 }, { 0, -6 },
//        };
//        for (auto& ep : enemyPos) {
//            Enemy e;
//            e.pos = {ep.x, 0.6f, ep.z};
//            gEnemies.push_back(e);
//        }
//    }
//#endif
//
//    gPlayerHP = gPlayerMaxHP;
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    gFrameCount++;
//    uint64_t now = get_ticks();
//    gDeltaTime = (now - gLastTime) / 1000.0f;   // get_ticks() 返回毫秒，转成秒
//    gLastTime = now;
//
//    processInput();
//    updateMVPData();
//
//    // ----- 子弹更新 -----
//    gShootTimer += gDeltaTime;
//    if (gKeys.shoot && gShootTimer >= gShootCooldown) {
//        gShootTimer = 0.0f;
//        float yawRad = ft_radians(gPlayerYaw);
//        vec3_t dir = {sinf(yawRad), 0, cosf(yawRad)};
//        Bullet b;
//        b.pos = vec3_add(vec3_add(gPlayerPos, {0, 0.4f, 0}), vec3_scale(dir, 0.6f));
//        b.dir = dir;
//        gBullets.push_back(b);
//    }
//
//    for (int i = (int)gBullets.size() - 1; i >= 0; i--) {
//        auto& b = gBullets[i];
//        b.age += gDeltaTime;
//        b.pos = vec3_add(b.pos, vec3_scale(b.dir, b.speed * gDeltaTime));
//
//        // 子弹超出边界 → 销毁
//        if (fabsf(b.pos.x) > 10.5f || fabsf(b.pos.z) > 10.5f) { gBullets.erase(gBullets.begin() + i); continue; }
//
//        // 子弹击中箱子 → 销毁
//        bool hitBox = false;
//        for (auto& box : gBoxes) {
//            gjk3df_collider_t bulShape;
//            gjk3df_init_box(&bulShape, toV3f(b.pos), toV3f({0.15f, 0.15f, 0.15f}));
//            vec3f_t gjkInitDir = { fp_one(), fp_zero(), fp_zero() };
//            gjk3df_contact_t c;
//            if (gjk3df_collide(&bulShape, &box.gjkShape, gjkInitDir, &c)) { hitBox = true; break; }
//        }
//        if (hitBox) { gBullets.erase(gBullets.begin() + i); continue; }
//
//        // 子弹击中敌人
//        bool hitEnemy = false;
//        for (auto& e : gEnemies) {
//            if (!e.alive) continue;
//            vec3_t diff = vec3_sub(b.pos, e.pos);
//            diff.y = 0;
//            if (vec3_length(diff) < 0.6f) {
//                e.hp -= b.damage;
//                if (e.hp <= 0) e.alive = false;
//                hitEnemy = true;
//                break;
//            }
//        }
//        if (hitEnemy) { gBullets.erase(gBullets.begin() + i); continue; }
//
//        // 超时
//        if (b.age > b.lifetime) { gBullets.erase(gBullets.begin() + i); continue; }
//    }
//
//
//
//    bool minimized = SDL_GetWindowFlags((SDL_Window*)get_raw_window(gWin)) & SDL_WINDOW_MINIMIZED;
//    if (minimized) return SDL_APP_CONTINUE;
//
//    if (!r3d_begin(gR3d)) return SDL_APP_CONTINUE;
//
//    // 相机（MOBA 俯视角：固定俯视，仅随鼠标水平旋转）
//    float yawRad = ft_radians(gCameraYaw);
//    float pitchRad = ft_radians(gPitch);
//    vec3_t camOffset = {-sinf(yawRad) * cosf(pitchRad) * CAM_DIST,
//                           sinf(pitchRad) * CAM_DIST,
//                           -cosf(yawRad) * cosf(pitchRad) * CAM_DIST};
//    vec3_t eye = vec3_add(gPlayerPos, camOffset);
//    vec3_t target = vec3_add(gPlayerPos, {0, 1.0f, 0});
//    r3d_camera_lookat(gR3d, eye, target, {0, 1, 0}, 60.0f);
//
//    // 光照：更新太阳方向与观察点
//    gLightBlock.viewPos[0] = eye.x; gLightBlock.viewPos[1] = eye.y; gLightBlock.viewPos[2] = eye.z;
//    float sy = sinf(yawRad), cy = cosf(yawRad);
//    gLightBlock.lights[0].target[0] = -sy;
//    gLightBlock.lights[0].target[1] = -1.0f;
//    gLightBlock.lights[0].target[2] = -cy;
//    r3d_set_light_block(gR3d, &gLightBlock);
//
//
//
//    // 绘制所有场景物体（地板用纹理，其余用颜色）
//    for (size_t i = 0; i < gDrawMeshes.size(); i++) {
//        auto& dm = gDrawMeshes[i];
//        if (dm.tex)
//            r3d_draw_mesh_lit(gR3d, dm.mesh, &gModelMatrices[i], dm.tex);
//        else
//            r3d_draw_mesh_lit_color(gR3d, dm.mesh, &gModelMatrices[i], dm.color);
//    }
//
//    /* 敌人已屏蔽*/
//    for (auto& e : gEnemies) {
//        if (!e.alive) continue;
//        mat44_t m = mat44_identity();
//        m.m0 = 0.5f; m.m5 = 0.5f; m.m10 = 0.5f;
//        m.m12 = e.pos.x; m.m13 = e.pos.y; m.m14 = e.pos.z;
//        r3d_draw_mesh_lit_color(gR3d, gUnitCubeMesh, &m, {220, 40, 40});
//    }
//
//    // 绘制子弹（黄色小立方体）
//    for (auto& b : gBullets) {
//        mat44_t m = mat44_identity();
//        m.m0 = 0.08f; m.m5 = 0.08f; m.m10 = 0.08f;
//        m.m12 = b.pos.x; m.m13 = b.pos.y; m.m14 = b.pos.z;
//        r3d_draw_mesh_lit_color(gR3d, gBulletMesh, &m, {255, 200, 50});
//    }
//
//    // 测试: 先渲染 logo.png 验证纹理路径(billboard, 位于玩家头顶上方)
//    if (gLogoTex) {
//        draw_image_3d(gLogoTex, gLogoTexW, gLogoTexH, {gPlayerPos.x, gPlayerPos.y + 6.0f, gPlayerPos.z}, 3.0f, yawRad, pitchRad);
//    } else if (gFrameCount == 0) {
//        SDL_Log("LOGO TEX IS NULL - 检查 joy_editor/logo.png 路径");
//    }
//
//    // 纯 r3d 字体界面: 在玩家头顶上方渲染一行文字(billboard 面向相机)
//    draw_text_3d("HELLO JOY 3D TEXT", {gPlayerPos.x, gPlayerPos.y + 3.0f, gPlayerPos.z}, 1.0f, yawRad, pitchRad);
//
//    r3d_end(gR3d);
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//
//    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
//        return SDL_APP_SUCCESS;
//    }
//
//    // 鼠标移动 → 水平旋转相机（MOBA 式拖拽转视角）
//    if (event->type == SDL_EVENT_MOUSE_MOTION) {
//        gCameraYaw += event->motion.xrel * 0.05f;
//    }
//
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    if (gR3d) {
//        r3d_mesh_destroy(gR3d, gFloorMesh);
//        // gPlayerMesh 指向 bakedMeshes 中某个, 不单独释放, 统一销毁烘焙序列避免双重释放
//        for (auto* mesh : gPlayerSkin.bakedMeshes) if (mesh) r3d_mesh_destroy(gR3d, mesh);
//        gPlayerSkin.bakedMeshes.clear();
//        gPlayerMesh = nullptr;
//        r3d_mesh_destroy(gR3d, gUnitCubeMesh);
//        r3d_mesh_destroy(gR3d, gSphereMesh);
//        r3d_mesh_destroy(gR3d, gBulletMesh);
//        r3d_mesh_destroy(gR3d, gRampMesh);
//        for (int i = 0; i < 128; i++) if (gFontGlyphMeshes[i]) { r3d_mesh_destroy(gR3d, gFontGlyphMeshes[i]); gFontGlyphMeshes[i] = nullptr; }
//        if (gPlayerSkin.loaded && gPlayerSkin.data) cgltf_free(gPlayerSkin.data);
//        r3d_destroy(gR3d);
//        gR3d = nullptr;
//    }
//    if (gWin) {
//        destroy_window(gWin);
//        gWin = nullptr;
//    }
//}
//
