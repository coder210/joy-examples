//// collision2d_test.cpp — 交互式碰撞检测测试
//// 编译: cmake -DBUILD_COLLISION2D_TEST=ON ..
//// 鼠标拖拽任意几何体,实时显示碰撞结果
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <cstdio>
//#include <cstdlib>
//#include <cmath>
//#include <cstring>
//#include <vector>
//#include <string>
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <joy/jcollision.h>
//#include <joy/jmath.h>
//
//#define M_PI 3.1415926
//
//// ======================== 常量 ========================
//static const int WINDOW_W = 1200, WINDOW_H = 800;
//static const float PPM = 100.0f;     // pixels per meter (FP→屏幕)
//
//// ======================== 全局状态 ========================
//static SDL_Window*   gWindow  = nullptr;
//static SDL_Renderer* gRenderer = nullptr;
//
//// ======================== 可拖拽形状 ========================
//enum ShapeKind {
//    SHAPE_CIRCLE,
//    SHAPE_RECT,       // AABB
//    SHAPE_RECT_ROT,   // OBB (带旋转)
//    SHAPE_POLYGON,    // 凸多边形
//    SHAPE_TRIANGLE,   // 三角形(3边多边形)
//    SHAPE_LINE,       // 线段(用于line-line以及point-line测试)
//};
//
//struct DragShape {
//    ShapeKind kind;
//    vec2f_t   pos;        // 位置/中心 (FP 坐标)
//    fp_t      radius;     // 圆半径
//    fp_t      w, h;       // 矩形宽高
//    fp_t      angle;      // 旋转角度(度, FP)
//    vec2f_t   verts[8];   // 多边形顶点(相对 pos 的偏移量)
//    int       num_verts;
//    vec2f_t   line_end;   // 线段终点(相对偏移)
//
//    // 运行时碰撞状态
//    bool    colliding;
//    vec2f_t contact_sp, contact_ep;
//    vec2f_t contact_normal;
//
//    // 拖拽状态
//    bool    dragging;
//    float   drag_off_x, drag_off_y; // 像素坐标下的偏移
//    char    label[32];
//};
//
//static std::vector<DragShape> gShapes;
//
//// ======================== 形状辅助 ========================
//static DragShape* hit_test(float mx, float my) {
//    for (auto& s : gShapes) {
//        float sx = fp_to_float(s.pos.x) * PPM;
//        float sy = fp_to_float(s.pos.y) * PPM;
//        // 粗略矩形包围盒测试
//        float half_w = 1.5f, half_h = 1.5f;
//        if (s.kind == SHAPE_CIRCLE) {
//            float r = fp_to_float(s.radius) * PPM + 5;
//            if (fabsf(mx - sx) < r && fabsf(my - sy) < r)
//                return &s;
//        } else if (s.kind == SHAPE_RECT || s.kind == SHAPE_RECT_ROT) {
//            half_w = fp_to_float(s.w) * PPM * 0.5f + 5;
//            half_h = fp_to_float(s.h) * PPM * 0.5f + 5;
//            if (fabsf(mx - sx) < half_w && fabsf(my - sy) < half_h)
//                return &s;
//        } else if (s.kind == SHAPE_POLYGON || s.kind == SHAPE_TRIANGLE) {
//            // 粗略点测试
//            float minx = 1e9, maxx = -1e9, miny = 1e9, maxy = -1e9;
//            for (int i = 0; i < s.num_verts; i++) {
//                float vx = fp_to_float(s.verts[i].x) * PPM + sx;
//                float vy = fp_to_float(s.verts[i].y) * PPM + sy;
//                if (vx < minx) minx = vx; if (vx > maxx) maxx = vx;
//                if (vy < miny) miny = vy; if (vy > maxy) maxy = vy;
//            }
//            if (mx >= minx-10 && mx <= maxx+10 && my >= miny-10 && my <= maxy+10)
//                return &s;
//        } else if (s.kind == SHAPE_LINE) {
//            float ex = sx + fp_to_float(s.line_end.x) * PPM;
//            float ey = sy + fp_to_float(s.line_end.y) * PPM;
//            float d = fabsf((ey-sy)*mx - (ex-sx)*my + ex*sy - ey*sx)
//                    / sqrtf((ex-sx)*(ex-sx)+(ey-sy)*(ey-sy));
//            if (d < 15) return &s;
//        }
//    }
//    return nullptr;
//}
//
//// ======================== 渲染函数 ========================
//static void draw_circle(float cx, float cy, float r, SDL_Color c, bool fill) {
//    SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
//    int segs = 48;
//    for (int i = 0; i < segs; i++) {
//        float a1 = (float)i / segs * 2 * M_PI;
//        float a2 = (float)(i+1) / segs * 2 * M_PI;
//        float x1 = cx + r * cosf(a1), y1 = cy + r * sinf(a1);
//        float x2 = cx + r * cosf(a2), y2 = cy + r * sinf(a2);
//        SDL_RenderLine(gRenderer, x1, y1, x2, y2);
//    }
//}
//
//static void draw_rect(float x, float y, float w, float h, SDL_Color c, float angle_deg) {
//    SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
//    float hw = w/2, hh = h/2;
//    float cos_a = cosf(angle_deg * M_PI / 180);
//    float sin_a = sinf(angle_deg * M_PI / 180);
//    float corners[4][2] = {{-hw,-hh},{hw,-hh},{hw,hh},{-hw,hh}};
//    float pts[4][2];
//    for (int i = 0; i < 4; i++) {
//        pts[i][0] = x + corners[i][0]*cos_a - corners[i][1]*sin_a;
//        pts[i][1] = y + corners[i][0]*sin_a + corners[i][1]*cos_a;
//    }
//    for (int i = 0; i < 4; i++) {
//        int j = (i+1)%4;
//        SDL_RenderLine(gRenderer, pts[i][0], pts[i][1], pts[j][0], pts[j][1]);
//    }
//}
//
//static void draw_polygon(const vec2f_t* verts, int n, float ox, float oy, SDL_Color c) {
//    SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
//    for (int i = 0; i < n; i++) {
//        int j = (i+1)%n;
//        float x1 = ox + fp_to_float(verts[i].x) * PPM;
//        float y1 = oy + fp_to_float(verts[i].y) * PPM;
//        float x2 = ox + fp_to_float(verts[j].x) * PPM;
//        float y2 = oy + fp_to_float(verts[j].y) * PPM;
//        SDL_RenderLine(gRenderer, x1, y1, x2, y2);
//    }
//}
//
//static void draw_shape(const DragShape& s) {
//    float sx = fp_to_float(s.pos.x) * PPM;
//    float sy = fp_to_float(s.pos.y) * PPM;
//    SDL_Color col = s.colliding ? SDL_Color{255,50,50,255} : SDL_Color{50,200,50,255};
//
//    switch (s.kind) {
//    case SHAPE_CIRCLE: {
//        float r = fp_to_float(s.radius) * PPM;
//        draw_circle(sx, sy, r, col, false);
//        // 圆心点
//        SDL_SetRenderDrawColor(gRenderer, 200,200,200,255);
//        SDL_RenderPoint(gRenderer, sx, sy);
//        break;
//    }
//    case SHAPE_RECT: {
//        float w = fp_to_float(s.w) * PPM;
//        float h = fp_to_float(s.h) * PPM;
//        SDL_FRect r = {sx - w/2, sy - h/2, w, h};
//        SDL_SetRenderDrawColor(gRenderer, col.r, col.g, col.b, col.a);
//        SDL_RenderRect(gRenderer, &r);
//        break;
//    }
//    case SHAPE_RECT_ROT: {
//        float w = fp_to_float(s.w) * PPM;
//        float h = fp_to_float(s.h) * PPM;
//        float deg = fp_to_float(s.angle);
//        draw_rect(sx, sy, w, h, col, deg);
//        break;
//    }
//    case SHAPE_POLYGON:
//    case SHAPE_TRIANGLE: {
//        draw_polygon(s.verts, s.num_verts, sx, sy, col);
//        break;
//    }
//    case SHAPE_LINE: {
//        float ex = sx + fp_to_float(s.line_end.x) * PPM;
//        float ey = sy + fp_to_float(s.line_end.y) * PPM;
//        SDL_SetRenderDrawColor(gRenderer, col.r, col.g, col.b, col.a);
//        SDL_RenderLine(gRenderer, sx, sy, ex, ey);
//        break;
//    }
//    }
//
//    // 碰撞法线/接触点
//    if (s.colliding) {
//        float nsx = fp_to_float(s.contact_sp.x) * PPM;
//        float nsy = fp_to_float(s.contact_sp.y) * PPM;
//        float nex = fp_to_float(s.contact_ep.x) * PPM;
//        float ney = fp_to_float(s.contact_ep.y) * PPM;
//        SDL_SetRenderDrawColor(gRenderer, 255,255,0,255);
//        SDL_RenderLine(gRenderer, nsx, nsy, nex, ney);
//        // 法线方向
//        float nx = fp_to_float(s.contact_normal.x) * 30;
//        float ny = fp_to_float(s.contact_normal.y) * 30;
//        float midx = (nsx+nex)/2, midy = (nsy+ney)/2;
//        SDL_SetRenderDrawColor(gRenderer, 0,200,255,255);
//        SDL_RenderLine(gRenderer, midx, midy, midx+nx, midy+ny);
//    }
//
//    // 标签
//    SDL_SetRenderDrawColor(gRenderer, 255,255,255,255);
//    // 简单文本直接用SDL_RenderDebugText (SDL3 有)
//    // 也可以用 SDL_RenderDebugText(gRenderer, sx-30, sy-35, s.label);
//    // 但 SDL3 中 DebugText 可能需要字体, 跳过文字渲染保持简单
//    // 用 title bar 显示信息代替
//}
//
//// ======================== 碰撞检测 ========================
//static void run_collision_tests() {
//    // 重置碰撞状态
//    for (auto& s : gShapes) {
//        s.colliding = false;
//        memset(&s.contact_sp, 0, sizeof(s.contact_sp));
//        memset(&s.contact_ep, 0, sizeof(s.contact_ep));
//        memset(&s.contact_normal, 0, sizeof(s.contact_normal));
//    }
//
//    // 对每对形状运行碰撞检测
//    for (size_t i = 0; i < gShapes.size(); i++) {
//        for (size_t j = i+1; j < gShapes.size(); j++) {
//            auto& a = gShapes[i];
//            auto& b = gShapes[j];
//
//            // 根据类型选择碰撞检测函数
//            if (a.kind == SHAPE_CIRCLE && b.kind == SHAPE_CIRCLE) {
//                circlef_t ca = {{a.pos.x, a.pos.y}, a.radius};
//                circlef_t cb = {{b.pos.x, b.pos.y}, b.radius};
//                contact2df_t ct;
//                if (collision2df_get_circles(ca, cb, &ct)) {
//                    a.colliding = b.colliding = true;
//                    a.contact_sp = ct.sp; b.contact_sp = ct.sp;
//                    a.contact_ep = ct.ep; b.contact_ep = ct.ep;
//                    a.contact_normal = ct.normal; b.contact_normal = ct.normal;
//                }
//            }
//            else if (a.kind == SHAPE_RECT && b.kind == SHAPE_RECT) {
//                fp_t half_wa = fp_mul(a.w, fp_half()), half_ha = fp_mul(a.h, fp_half());
//                fp_t half_wb = fp_mul(b.w, fp_half()), half_hb = fp_mul(b.h, fp_half());
//                rectanglef_t ra = {fp_sub(a.pos.x, half_wa), fp_sub(a.pos.y, half_ha), a.w, a.h};
//                rectanglef_t rb = {fp_sub(b.pos.x, half_wb), fp_sub(b.pos.y, half_hb), b.w, b.h};
//                if (collision2df_check_rectangles(ra, rb)) {
//                    a.colliding = b.colliding = true;
//                }
//            }
//            else if ((a.kind == SHAPE_RECT_ROT || a.kind == SHAPE_RECT) &&
//                     (b.kind == SHAPE_RECT_ROT || b.kind == SHAPE_RECT)) {
//                fp_t half_wa = fp_mul(a.w, fp_half()), half_ha = fp_mul(a.h, fp_half());
//                fp_t half_wb = fp_mul(b.w, fp_half()), half_hb = fp_mul(b.h, fp_half());
//                rectanglef_t ra = {fp_sub(a.pos.x, half_wa), fp_sub(a.pos.y, half_ha), a.w, a.h};
//                rectanglef_t rb = {fp_sub(b.pos.x, half_wb), fp_sub(b.pos.y, half_hb), b.w, b.h};
//                fp_t aa = (a.kind == SHAPE_RECT_ROT) ? a.angle : 0;
//                fp_t ab = (b.kind == SHAPE_RECT_ROT) ? b.angle : 0;
//                contact2df_t ct;
//                if (collision2df_get_rectangles(ra, aa, rb, ab, &ct)) {
//                    a.colliding = b.colliding = true;
//                    a.contact_sp = ct.sp; b.contact_sp = ct.sp;
//                    a.contact_ep = ct.ep; b.contact_ep = ct.ep;
//                    a.contact_normal = ct.normal; b.contact_normal = ct.normal;
//                }
//            }
//            else if ((a.kind == SHAPE_POLYGON || a.kind == SHAPE_TRIANGLE) &&
//                     (b.kind == SHAPE_POLYGON || b.kind == SHAPE_TRIANGLE)) {
//                // 构建全局坐标下的多边形
//                vec2f_t a_global[8], b_global[8];
//                for (int k = 0; k < a.num_verts; k++) {
//                    a_global[k] = vec2f_add(a.pos, a.verts[k]);
//                }
//                for (int k = 0; k < b.num_verts; k++) {
//                    b_global[k] = vec2f_add(b.pos, b.verts[k]);
//                }
//                polygonf_t pa = {a_global, a.num_verts};
//                polygonf_t pb = {b_global, b.num_verts};
//                contact2df_t ct;
//                if (collision2df_get_polygons(pa, pb, &ct)) {
//                    a.colliding = b.colliding = true;
//                    a.contact_sp = ct.sp; b.contact_sp = ct.sp;
//                    a.contact_ep = ct.ep; b.contact_ep = ct.ep;
//                    a.contact_normal = ct.normal; b.contact_normal = ct.normal;
//                }
//            }
//            else if (a.kind == SHAPE_CIRCLE && b.kind == SHAPE_RECT) {
//                circlef_t ca = {{a.pos.x, a.pos.y}, a.radius};
//                fp_t half_w = fp_mul(b.w, fp_half()), half_h = fp_mul(b.h, fp_half());
//                rectanglef_t rb = {fp_sub(b.pos.x, half_w), fp_sub(b.pos.y, half_h), b.w, b.h};
//                if (collision2df_check_circle_rectangle(ca, rb)) {
//                    a.colliding = b.colliding = true;
//                }
//            }
//            else if (a.kind == SHAPE_RECT && b.kind == SHAPE_CIRCLE) {
//                circlef_t cb = {{b.pos.x, b.pos.y}, b.radius};
//                fp_t half_w = fp_mul(a.w, fp_half()), half_h = fp_mul(a.h, fp_half());
//                rectanglef_t ra = {fp_sub(a.pos.x, half_w), fp_sub(a.pos.y, half_h), a.w, a.h};
//                if (collision2df_check_circle_rectangle(cb, ra)) {
//                    a.colliding = b.colliding = true;
//                }
//            }
//            else if (a.kind == SHAPE_CIRCLE && (b.kind == SHAPE_POLYGON || b.kind == SHAPE_TRIANGLE)) {
//                circlef_t ca = {{a.pos.x, a.pos.y}, a.radius};
//                vec2f_t b_global[8];
//                for (int k = 0; k < b.num_verts; k++)
//                    b_global[k] = vec2f_add(b.pos, b.verts[k]);
//                polygonf_t pb = {b_global, b.num_verts};
//                if (collision2df_check_circle_polygon(ca, pb)) {
//                    a.colliding = b.colliding = true;
//                }
//            }
//            else if ((a.kind == SHAPE_POLYGON || a.kind == SHAPE_TRIANGLE) && b.kind == SHAPE_CIRCLE) {
//                circlef_t cb = {{b.pos.x, b.pos.y}, b.radius};
//                vec2f_t a_global[8];
//                for (int k = 0; k < a.num_verts; k++)
//                    a_global[k] = vec2f_add(a.pos, a.verts[k]);
//                polygonf_t pa = {a_global, a.num_verts};
//                if (collision2df_check_circle_polygon(cb, pa)) {
//                    a.colliding = b.colliding = true;
//                }
//            }
//        }
//    }
//}
//
//// ======================== 场景初始化 ========================
//static void init_scene() {
//    gShapes.clear();
//
//    // 1. Circle vs Circle
//    {
//        DragShape s;
//        s.kind = SHAPE_CIRCLE;
//        s.pos = {fp_from_float(2), fp_from_float(2)};
//        s.radius = fp_from_float(1.0f);
//        s.dragging = false;
//        strcpy(s.label, "Circle-A");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_CIRCLE;
//        s.pos = {fp_from_float(4.5f), fp_from_float(2)};
//        s.radius = fp_from_float(0.8f);
//        s.dragging = false;
//        strcpy(s.label, "Circle-B");
//        gShapes.push_back(s);
//    }
//
//    // 2. Rectangle vs Rectangle (AABB)
//    {
//        DragShape s;
//        s.kind = SHAPE_RECT;
//        s.pos = {fp_from_float(2), fp_from_float(5)};
//        s.w = fp_from_float(2.0f); s.h = fp_from_float(1.5f);
//        s.dragging = false;
//        strcpy(s.label, "Rect-A");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_RECT;
//        s.pos = {fp_from_float(4), fp_from_float(5.5f)};
//        s.w = fp_from_float(1.5f); s.h = fp_from_float(1.0f);
//        s.dragging = false;
//        strcpy(s.label, "Rect-B");
//        gShapes.push_back(s);
//    }
//
//    // 3. Rotated Rect vs Rotated Rect (OBB)
//    {
//        DragShape s;
//        s.kind = SHAPE_RECT_ROT;
//        s.pos = {fp_from_float(8), fp_from_float(2)};
//        s.w = fp_from_float(2.0f); s.h = fp_from_float(1.0f);
//        s.angle = fp_from_float(15);
//        s.dragging = false;
//        strcpy(s.label, "OBB-A");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_RECT_ROT;
//        s.pos = {fp_from_float(10), fp_from_float(2.5f)};
//        s.w = fp_from_float(1.5f); s.h = fp_from_float(1.5f);
//        s.angle = fp_from_float(30);
//        s.dragging = false;
//        strcpy(s.label, "OBB-B");
//        gShapes.push_back(s);
//    }
//
//    // 4. Polygon vs Polygon
//    {
//        DragShape s;
//        s.kind = SHAPE_POLYGON;
//        s.pos = {fp_from_float(7), fp_from_float(5.5f)};
//        s.num_verts = 4;
//        s.verts[0] = {fp_from_float(-0.8f), fp_from_float(-0.8f)};
//        s.verts[1] = {fp_from_float(0.8f),  fp_from_float(-0.6f)};
//        s.verts[2] = {fp_from_float(0.6f),  fp_from_float(0.8f)};
//        s.verts[3] = {fp_from_float(-0.6f), fp_from_float(0.8f)};
//        s.dragging = false;
//        strcpy(s.label, "Poly-A");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_POLYGON;
//        s.pos = {fp_from_float(9.5f), fp_from_float(5.5f)};
//        s.num_verts = 4;
//        s.verts[0] = {fp_from_float(-0.6f), fp_from_float(-0.8f)};
//        s.verts[1] = {fp_from_float(0.8f),  fp_from_float(-0.6f)};
//        s.verts[2] = {fp_from_float(0.7f),  fp_from_float(0.7f)};
//        s.verts[3] = {fp_from_float(-0.7f), fp_from_float(0.6f)};
//        s.dragging = false;
//        strcpy(s.label, "Poly-B");
//        gShapes.push_back(s);
//    }
//
//    // 5. Triangle vs Triangle
//    {
//        DragShape s;
//        s.kind = SHAPE_TRIANGLE;
//        s.pos = {fp_from_float(12), fp_from_float(5)};
//        s.num_verts = 3;
//        s.verts[0] = {fp_from_float(-0.7f), fp_from_float(-0.5f)};
//        s.verts[1] = {fp_from_float(0.7f),  fp_from_float(-0.5f)};
//        s.verts[2] = {fp_from_float(0),     fp_from_float(0.7f)};
//        s.dragging = false;
//        strcpy(s.label, "Tri-A");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_TRIANGLE;
//        s.pos = {fp_from_float(13.5f), fp_from_float(5)};
//        s.num_verts = 3;
//        s.verts[0] = {fp_from_float(-0.5f), fp_from_float(-0.6f)};
//        s.verts[1] = {fp_from_float(0.6f),  fp_from_float(-0.6f)};
//        s.verts[2] = {fp_from_float(0.05f), fp_from_float(0.6f)};
//        s.dragging = false;
//        strcpy(s.label, "Tri-B");
//        gShapes.push_back(s);
//    }
//
//    // 6. Circle vs Rectangle
//    {
//        DragShape s;
//        s.kind = SHAPE_CIRCLE;
//        s.pos = {fp_from_float(2), fp_from_float(8)};
//        s.radius = fp_from_float(0.7f);
//        s.dragging = false;
//        strcpy(s.label, "CIR-RECT");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_RECT;
//        s.pos = {fp_from_float(4), fp_from_float(8)};
//        s.w = fp_from_float(1.8f); s.h = fp_from_float(1.2f);
//        s.dragging = false;
//        strcpy(s.label, "");
//        gShapes.push_back(s);
//    }
//
//    // 7. Circle vs Polygon
//    {
//        DragShape s;
//        s.kind = SHAPE_CIRCLE;
//        s.pos = {fp_from_float(7), fp_from_float(8)};
//        s.radius = fp_from_float(0.6f);
//        s.dragging = false;
//        strcpy(s.label, "CIR-POLY");
//        gShapes.push_back(s);
//    }
//    {
//        DragShape s;
//        s.kind = SHAPE_POLYGON;
//        s.pos = {fp_from_float(9), fp_from_float(8)};
//        s.num_verts = 4;
//        s.verts[0] = {fp_from_float(-0.6f), fp_from_float(-0.6f)};
//        s.verts[1] = {fp_from_float(0.7f),  fp_from_float(-0.5f)};
//        s.verts[2] = {fp_from_float(0.5f),  fp_from_float(0.7f)};
//        s.verts[3] = {fp_from_float(-0.5f), fp_from_float(0.6f)};
//        s.dragging = false;
//        strcpy(s.label, "");
//        gShapes.push_back(s);
//    }
//}
//
//// ======================== SDL3 回调 ========================
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
//    (void)argc; (void)argv;
//    *appstate = nullptr;
//
//    if (!SDL_CreateWindowAndRenderer("Collision2D Interactive Test",
//            WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE, &gWindow, &gRenderer)) {
//        SDL_Log("SDL_Window: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//    init_scene();
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    (void)appstate;
//    if (event->type == SDL_EVENT_QUIT)
//        return SDL_APP_SUCCESS;
//    if (event->type == SDL_EVENT_KEY_DOWN) {
//        if (event->key.scancode == SDL_SCANCODE_ESCAPE ||
//            event->key.scancode == SDL_SCANCODE_Q) {
//            return SDL_APP_SUCCESS;
//        }
//        if (event->key.scancode == SDL_SCANCODE_R) {
//            init_scene();
//            return SDL_APP_CONTINUE;
//        }
//    }
//
//    float mx = event->button.x, my = event->button.y;
//    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
//        DragShape* hit = hit_test(mx, my);
//        if (hit) {
//            hit->dragging = true;
//            hit->drag_off_x = mx - fp_to_float(hit->pos.x) * PPM;
//            hit->drag_off_y = my - fp_to_float(hit->pos.y) * PPM;
//        }
//    }
//    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
//        for (auto& s : gShapes) s.dragging = false;
//    }
//    if (event->type == SDL_EVENT_MOUSE_MOTION) {
//        for (auto& s : gShapes) {
//            if (s.dragging) {
//                s.pos.x = fp_from_float((event->motion.x - s.drag_off_x) / PPM);
//                s.pos.y = fp_from_float((event->motion.y - s.drag_off_y) / PPM);
//            }
//        }
//    }
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    // 碰撞检测
//    run_collision_tests();
//
//    // 渲染
//    SDL_SetRenderDrawColor(gRenderer, 20, 20, 30, 255);
//    SDL_RenderClear(gRenderer);
//
//    // 坐标网格
//    SDL_SetRenderDrawColor(gRenderer, 40, 40, 50, 255);
//    for (float gx = 0; gx < 20; gx += 1) {
//        float px = gx * PPM;
//        SDL_RenderLine(gRenderer, px, 0, px, WINDOW_H);
//    }
//    for (float gy = 0; gy < 15; gy += 1) {
//        float py = gy * PPM;
//        SDL_RenderLine(gRenderer, 0, py, WINDOW_W, py);
//    }
//
//    // 绘制形状
//    for (auto& s : gShapes) draw_shape(s);
//
//    // 窗口标题显示碰撞状态
//    int coll_count = 0;
//    for (auto& s : gShapes) if (s.colliding) coll_count++;
//    {
//        char title[128];
//        snprintf(title, sizeof(title),
//            "Collision2D Test | %zu shapes | %d colliding | Drag: LMB | Reset: R",
//            gShapes.size(), coll_count);
//        SDL_SetWindowTitle(gWindow, title);
//    }
//
//    SDL_RenderPresent(gRenderer);
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult res) {
//    (void)appstate;
//    if (gRenderer) SDL_DestroyRenderer(gRenderer);
//    if (gWindow) SDL_DestroyWindow(gWindow);
//}
