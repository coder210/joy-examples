// #define SDL_MAIN_USE_CALLBACKS
// #include <SDL3/SDL.h>
// #include <SDL3/SDL_main.h>
// #include "gjk2d.h"
// #include <joy/jgjk.h>
// #include <cstdio>
// #include <cmath>
// #include <memory>
// #include <vector>

// using namespace gjk2d;

// // ============================================================================
// // 原始单元测试（来自原 main9.cpp）
// // ============================================================================
// static int tests_passed = 0, tests_failed = 0;

// static void tcheck(const char* name, bool expected, bool actual) {
//     if (expected == actual) { SDL_Log("  [PASS] %s", name); tests_passed++; }
//     else { SDL_Log("  [FAIL] %s (expected=%d, actual=%d)", name, expected, actual); tests_failed++; }
// }

// static void tcheck_pen(const char* name, bool exp_hit, bool act_hit,
//                        const Penetration& pen, float lo, float hi, float nx, float ny) {
//     if (exp_hit != act_hit) { SDL_Log("  [FAIL] %s", name); tests_failed++; return; }
//     if (!act_hit) { SDL_Log("  [PASS] %s", name); tests_passed++; return; }
//     bool dok = pen.depth >= lo-0.01f && pen.depth <= hi+0.01f;
//     bool nok = std::abs(pen.normal.x-nx)<0.05f && std::abs(pen.normal.y-ny)<0.05f;
//     if (dok && nok) { SDL_Log("  [PASS] %s", name); tests_passed++; }
//     else { SDL_Log("  [FAIL] %s", name); tests_failed++; }
// }

// static void test_circle_math() {
//     SDL_Log("\n--- Circle-Circle ---");
//     auto c1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(2.0f)));
//     auto c2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(1.5f)));
//     Transform t1, t2; t1.position = {0,0};
//     t2.position = {5,0}; tcheck("d=5 r_sum=3.5", false, GJK::Detect(c1.get(),t1,c2.get(),t2));
//     t2.position = {3.5f,0}; tcheck("d=3.5 touching", true, GJK::Detect(c1.get(),t1,c2.get(),t2));
//     t2.position = {2,0}; Penetration p; bool h=GJK::Detect(c1.get(),t1,c2.get(),t2,p);
//     tcheck_pen("d=2 ov=1.5", true, h, p, 1.5f,1.5f,1.0f,0.0f);
//     t2.position = {2,2}; h=GJK::Detect(c1.get(),t1,c2.get(),t2,p);
//     float ed=3.5f-std::sqrt(8.0f), en=2.0f/std::sqrt(8.0f);
//     tcheck_pen("d=2.828 ov=0.672", true, h, p, ed-0.05f,ed+0.05f, en, en);
// }

// static void test_rect_math() {
//     SDL_Log("\n--- Rectangle ---");
//     auto r = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(4,2)));
//     Transform t; t.position = {0,0};
//     tcheck("+x", true, std::abs(r->GetFarthestProjectionPoint(t,{1,0}).x-2.0f)<0.01f);
//     tcheck("-x", true, std::abs(r->GetFarthestProjectionPoint(t,{-1,0}).x+2.0f)<0.01f);
// }

// static void test_exhaustive_rect_rect() {
//     SDL_Log("\n--- Rect-Rect exhaustive ---");
//     auto r1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(2,2)));
//     auto r2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(2,2)));
//     Transform t1,t2; t1.position={0,0};
//     struct {float x,y;bool h;const char*d;} cs[]={
//         {0,0,1,"center"},{1,0,1,"edge x=1"},{2.1f,0,0,"x=2.1"},{0,1,1,"edge y=1"},
//         {1,1,1,"corner(1,1)"},{2,0,0,"touch x=2"},{3,3,0,"far(3,3)"},{0.5f,0.5f,1,"overlap"},
//         {-1.5f,0,1,"neg x"},
//     };
//     for(auto&c:cs){t2.position={c.x,c.y};tcheck(c.d,c.h,GJK::Detect(r1.get(),t1,r2.get(),t2));}
// }

// static void test_rotation() {
//     SDL_Log("\n--- Rotation ---");
//     auto r1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(2,4)));
//     auto r2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(4,2)));
//     Transform t1,t2; t1.position={0,0}; t2.position={2,0};
//     tcheck("unrotated", true, GJK::Detect(r1.get(),t1,r2.get(),t2));
//     t1.rotation=90; tcheck("rot90", true, GJK::Detect(r1.get(),t1,r2.get(),t2));
//     t2.position={4,0}; tcheck("sep after rot", false, GJK::Detect(r1.get(),t1,r2.get(),t2));
// }

// static void test_ellipse() {
//     SDL_Log("\n--- Ellipse ---");
//     auto e = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Ellipse(4,2)));
//     auto c = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(0.5f)));
//     Transform t1,t2; t1.position={0,0};
//     t2.position={0,0}; tcheck("circle inside", true, GJK::Detect(e.get(),t1,c.get(),t2));
//     t2.position={5,0}; tcheck("circle far", false, GJK::Detect(e.get(),t1,c.get(),t2));
// }

// static void test_segment() {
//     SDL_Log("\n--- Segment ---");
//     auto s = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Segment(2,{0,1})));
//     auto c = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(0.5f)));
//     Transform t1,t2; t1.position={0,0};
//     t2.position={0,0}; tcheck("center", true, GJK::Detect(s.get(),t1,c.get(),t2));
//     t2.position={3,0}; tcheck("far", false, GJK::Detect(s.get(),t1,c.get(),t2));
// }

// static void test_polygon_directions() {
//     SDL_Log("\n--- Polygon ---");
//     std::vector<Vector2> v={{-2,-1},{2,-1},{2,1},{-2,1}}; Polygon poly(v);
//     tcheck("+x", true, std::abs(poly.GetFarthestProjectionPoint({1,0}).x-2.0f)<0.01f);
//     tcheck("-x", true, std::abs(poly.GetFarthestProjectionPoint({-1,0}).x+2.0f)<0.01f);
// }

// static void test_negative_coords() {
//     SDL_Log("\n--- Negative coordinates ---");
//     auto c1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(1)));
//     auto c2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(1)));
//     Transform t1,t2; t1.position={-5,-5};
//     t2.position={-4.5f,-4.5f}; tcheck("overlap", true, GJK::Detect(c1.get(),t1,c2.get(),t2));
//     t2.position={-2,-5}; tcheck("sep x", false, GJK::Detect(c1.get(),t1,c2.get(),t2));
// }

// static void test_epa_normal() {
//     SDL_Log("\n--- EPA normal ---");
//     auto r1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(2,2)));
//     auto r2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Rectangle(2,2)));
//     Transform t1,t2; t1.position={0,0}; Penetration p;
//     t2.position={1.5f,0}; GJK::Detect(r1.get(),t1,r2.get(),t2,p);
//     tcheck("normal +x", true, p.normal.x>0.9f&&std::abs(p.normal.y)<0.1f);
//     t2.position={0,1.5f}; GJK::Detect(r1.get(),t1,r2.get(),t2,p);
//     tcheck("normal +y", true, p.normal.y>0.9f&&std::abs(p.normal.x)<0.1f);
// }

// static void test_large_sep() {
//     SDL_Log("\n--- Large separation ---");
//     auto c1 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(0.5f)));
//     auto c2 = std::unique_ptr<ICollider>(ColliderFactory::CreateCollider(Circle(0.5f)));
//     Transform t1,t2; t1.position={0,0};
//     t2.position={10000,0}; tcheck("far x", false, GJK::Detect(c1.get(),t1,c2.get(),t2));
//     t2.position={10000,10000}; tcheck("far diag", false, GJK::Detect(c1.get(),t1,c2.get(),t2));
// }

// static void run_all_tests() {
//     SDL_Log("========== GJK2D Comprehensive Tests ==========");
//     tests_passed=tests_failed=0;
//     test_circle_math(); test_rect_math(); test_exhaustive_rect_rect();
//     test_rotation(); test_ellipse(); test_segment();
//     test_polygon_directions(); test_negative_coords();
//     test_epa_normal(); test_large_sep();
//     SDL_Log("========== Results: %d passed, %d failed ==========", tests_passed, tests_failed);
// }

// // ============================================================================
// // 2D 场景 — 类似 main8.cpp 的 GJK 3D 测试场景
// // ============================================================================
// #define WW 1024
// #define WH 720
// #define GS 40
// #define CX (WW/2)
// #define CY (WH/2)

// static float wx(float x) { return CX + x * GS; }
// static float wy(float y) { return CY - y * GS; }

// // 玩家（胶囊体）
// static constexpr float PLAYER_CAP_LEN = 0.6f;
// static constexpr float PLAYER_CAP_RAD = 0.3f;
// static Vector2 gPlayerPos = {0, 0};
// static Vector2 gPlayerVel = {0, 0};

// // 障碍物类型
// enum class ObsType { Circle, Rect, Poly, Capsule, Pie, Segment };

// struct Obstacle {
//     std::unique_ptr<ICollider> collider;  // 仅用于渲染
//     Transform transform;
//     float cr, cg, cb; // color RGB
//     ObsType type;
//     gjk2d_collider_t gjkShape;            // 定点数碰撞体(C API用)
//     vec2f_t gjkVerts[12];                 // 预分配顶点存储(多边形/矩形)
// };

// static vec2f_t toV2f(const Vector2& v) { return { fp_from_float(v.x), fp_from_float(v.y) }; }

// static std::vector<Obstacle> gObstacles;

// static void create_scene() {
//     gObstacles.clear();
//     gObstacles.reserve(32);  // 预分配,避免push_back时重分配导致gjkShape.vertices指针悬空
//     gPlayerPos = {-8, 0};

//     // 类似 main8.cpp 的障碍物布局，多个形状分布在地图各处
//     struct { float x, y; float p1, p2; float cr, cg, cb; ObsType t; } defs[] = {
//         // 圆形障碍物
//         {5, 0, 1.0f,  0, 0.3f, 0.8f, 0.3f, ObsType::Circle},
//         {-2, 4, 0.8f, 0, 0.8f, 0.3f, 0.3f, ObsType::Circle},
//         {4, -4, 0.6f, 0, 0.3f, 0.3f, 0.8f, ObsType::Circle},
//         // 矩形障碍物
//         {-3, -3, 2, 2, 0.2f, 0.7f, 0.7f, ObsType::Rect},
//         {7, 3, 3, 1, 0.7f, 0.5f, 0.2f, ObsType::Rect},
//         {0, -5, 1.5f, 1.5f, 0.8f, 0.8f, 0.2f, ObsType::Rect},
//         // 多边形障碍物（不规则凸多边形）
//         { -6, 2, 0, 0, 0.9f, 0.4f, 0.1f, ObsType::Poly },
//         { 3, -2, 0, 0, 0.5f, 0.2f, 0.7f, ObsType::Poly },
//         // 胶囊体障碍物
//         { -5, -2, 2.0f, 0.6f, 0.1f, 0.6f, 0.4f, ObsType::Capsule },
//         { 6, -2, 1.5f, 0.4f, 0.7f, 0.2f, 0.5f, ObsType::Capsule },
//         // 扇形障碍物
//         { 8, 5, 1.5f, 120, 0.9f, 0.6f, 0.1f, ObsType::Pie },
//         // 线段障碍物
//         { -8, 5, 3.0f, 0, 0.3f, 0.4f, 0.9f, ObsType::Segment },
//     };

//     for (auto& d : defs) {
//         Obstacle obs;
//         obs.transform.position = {d.x, d.y};
//         obs.transform.rotation = float(rand() % 360); // 随机旋转
//         obs.cr = d.cr; obs.cg = d.cg; obs.cb = d.cb;
//         obs.type = d.t;
//         memset(&obs.gjkShape, 0, sizeof(obs.gjkShape));

//         // 旋转辅助: 局部点 → 世界坐标
//         float rad = obs.transform.rotation * MathX::DEG2RAD;
//         float cr_ = cosf(rad), sr_ = sinf(rad);
//         vec2f_t pos_v2f = toV2f(obs.transform.position);
//         auto rot_v2f = [&](float lx, float ly) -> vec2f_t {
//             return { fp_from_float(lx * cr_ - ly * sr_ + obs.transform.position.x),
//                      fp_from_float(lx * sr_ + ly * cr_ + obs.transform.position.y) };
//         };

//         switch (d.t) {
//         case ObsType::Circle: {
//             obs.collider.reset(ColliderFactory::CreateCollider(Circle(d.p1)));
//             gjk2d_init_circle(&obs.gjkShape, pos_v2f, fp_from_float(d.p1));
//             break;
//         }
//         case ObsType::Rect: {
//             obs.collider.reset(ColliderFactory::CreateCollider(Rectangle(d.p1, d.p2)));
//             float hw = d.p1 * 0.5f, hh = d.p2 * 0.5f;
//             obs.gjkVerts[0] = rot_v2f(-hw, -hh);
//             obs.gjkVerts[1] = rot_v2f( hw, -hh);
//             obs.gjkVerts[2] = rot_v2f( hw,  hh);
//             obs.gjkVerts[3] = rot_v2f(-hw,  hh);
//             gjk2d_init_polygon(&obs.gjkShape, obs.gjkVerts, 4);
//             break;
//         }
//         case ObsType::Poly: {
//             std::vector<Vector2> verts = {
//                 {-1.2f, -0.8f}, {0.8f, -1.2f}, {1.3f, 0.2f}, {0.5f, 1.0f}, {-1.0f, 0.8f}
//             };
//             obs.collider.reset(ColliderFactory::CreateCollider(Polygon(verts)));
//             int nv = (int)verts.size() < 12 ? (int)verts.size() : 12;
//             for (int i = 0; i < nv; i++) obs.gjkVerts[i] = rot_v2f(verts[i].x, verts[i].y);
//             gjk2d_init_polygon(&obs.gjkShape, obs.gjkVerts, nv);
//             break;
//         }
//         case ObsType::Capsule: {
//             obs.collider.reset(ColliderFactory::CreateCollider(Capsule(d.p1, d.p2)));
//             vec2f_t axis = {fp_from_float(cr_), fp_from_float(sr_)};
//             gjk2d_init_capsule(&obs.gjkShape, pos_v2f, axis, fp_from_float(d.p1), fp_from_float(d.p2));
//             break;
//         }
//         case ObsType::Pie: {
//             obs.collider.reset(ColliderFactory::CreateCollider(Pie(d.p1, d.p2)));
//             // Pie 作为多边形近似(最多11个顶点: 10弧点+原点)
//             int np = 10, vi = 0;
//             float sweep_r = d.p2 * MathX::DEG2RAD;
//             for (int i = 0; i < np && vi < 11; i++) {
//                 float t = -sweep_r * 0.5f + (float)i / (np - 1) * sweep_r;
//                 obs.gjkVerts[vi++] = rot_v2f(cosf(t) * d.p1, sinf(t) * d.p1);
//             }
//             obs.gjkVerts[vi++] = rot_v2f(0, 0);
//             gjk2d_init_polygon(&obs.gjkShape, obs.gjkVerts, vi);
//             break;
//         }
//         case ObsType::Segment:
//             obs.collider.reset(ColliderFactory::CreateCollider(Segment(d.p1, Vector2(0,1))));
//             // 线段视为薄胶囊
//             {
//                 vec2f_t axis = {fp_from_float(cr_), fp_from_float(sr_)};
//                 gjk2d_init_capsule(&obs.gjkShape, pos_v2f, axis, fp_from_float(d.p1), fp_from_float(0.05f));
//             }
//             break;
//         }
//         gObstacles.push_back(std::move(obs));
//         // 修复: push_back 后 gjkShape.vertices 可能指向原栈 obs.gjkVerts
//         // 重定向到 vector 中实际存储的 gjkVerts
//         gObstacles.back().gjkShape.vertices = gObstacles.back().gjkVerts;
//     }
// }

// // ============================================================================
// // 绘制
// // ============================================================================
// static void draw_grid(SDL_Renderer* r) {
//     SDL_SetRenderDrawColor(r, 20, 20, 30, 255); SDL_RenderClear(r);
//     SDL_SetRenderDrawColor(r, 35, 35, 45, 255);
//     for (int x = CX % GS; x < WW; x += GS) SDL_RenderLine(r, (float)x, 0, (float)x, (float)WH);
//     for (int y = CY % GS; y < WH; y += GS) SDL_RenderLine(r, 0, (float)y, (float)WW, (float)y);
//     SDL_SetRenderDrawColor(r, 50, 200, 50, 200);
//     SDL_RenderLine(r, (float)CX, 0, (float)CX, (float)WH);
//     SDL_RenderLine(r, 0, (float)CY, (float)WW, (float)CY);
// }

// static void draw_circle_shape(SDL_Renderer* r, float cx, float cy, float rad,
//                                uint8_t r_, uint8_t g, uint8_t b, uint8_t a) {
//     float sr = rad * GS; int segs = (int)(sr * 2); if (segs < 20) segs = 20;
//     std::vector<SDL_FPoint> pts(segs + 1);
//     for (int i = 0; i <= segs; i++) {
//         float ang = (float)i / segs * 6.2831853f;
//         pts[i] = { cx + cosf(ang) * sr, cy + sinf(ang) * sr };
//     }
//     SDL_SetRenderDrawColor(r, r_, g, b, a); SDL_RenderLines(r, pts.data(), segs + 1);
// }

// static void draw_poly_shape(SDL_Renderer* r, const std::vector<Vector2>& verts,
//                              uint8_t r_, uint8_t g, uint8_t b, uint8_t a) {
//     if (verts.size() < 2) return;
//     int n = (int)verts.size();
//     std::vector<SDL_FPoint> pts(n + 1);
//     for (int i = 0; i < n; i++) pts[i] = { wx(verts[i].x), wy(verts[i].y) };
//     pts[n] = pts[0];
//     SDL_SetRenderDrawColor(r, r_, g, b, a); SDL_RenderLines(r, pts.data(), n + 1);
// }

// static std::vector<Vector2> get_world_verts(const ICollider* c, const Transform& t) {
//     std::vector<Vector2> res;
//     auto rot = Matrix::CreateRotationMatrix(t.rotation * MathX::DEG2RAD);
//     auto mat = rot * Matrix::CreateTranslationMatrix(t.position);
//     auto xf = [&](const Vector2& v) { return Matrix::Transform(v, mat); };
//     switch (c->geometryType()) {
//     case GeometryType::Rectangle: {
//         float w = static_cast<const BaseCollider<Rectangle>*>(c)->geometry.width * 0.5f;
//         float h = static_cast<const BaseCollider<Rectangle>*>(c)->geometry.height * 0.5f;
//         res = { xf({-w,-h}), xf({w,-h}), xf({w,h}), xf({-w,h}) }; break;
//     }
//     case GeometryType::Polygon:
//         for (auto& v : static_cast<const BaseCollider<Polygon>*>(c)->geometry.verts) res.push_back(xf(v));
//         break;
//     case GeometryType::Capsule: {
//         float hl = static_cast<const BaseCollider<Capsule>*>(c)->geometry.length * 0.5f;
//         res = { xf({hl,0}), xf({-hl,0}) }; break;
//     }
//     case GeometryType::Segment: {
//         float hl = static_cast<const BaseCollider<Segment>*>(c)->geometry.length * 0.5f;
//         res = { xf({hl,0}), xf({-hl,0}) }; break;
//     }
//     default: break;
//     }
//     return res;
// }

// static void draw_obstacle(SDL_Renderer* r, const Obstacle& obs, bool hit) {
//     uint8_t mul = hit ? 255 : 180;
//     uint8_t red   = (uint8_t)(obs.cr * mul);
//     uint8_t grn   = (uint8_t)(obs.cg * mul);
//     uint8_t blu   = (uint8_t)(obs.cb * mul);

//     switch (obs.type) {
//     case ObsType::Circle: {
//         float rad = static_cast<const BaseCollider<Circle>*>(obs.collider.get())->geometry.radius;
//         float sx = wx(obs.transform.position.x), sy = wy(obs.transform.position.y);
//         draw_circle_shape(r, sx, sy, rad, red/2, grn/2, blu/2, 80);
//         draw_circle_shape(r, sx, sy, rad, red, grn, blu, 220);
//         break;
//     }
//     case ObsType::Rect:
//     case ObsType::Poly: {
//         auto verts = get_world_verts(obs.collider.get(), obs.transform);
//         draw_poly_shape(r, verts, red/2, grn/2, blu/2, 80);
//         draw_poly_shape(r, verts, red, grn, blu, 220);
//         break;
//     }
//     case ObsType::Capsule: {
//         auto v = get_world_verts(obs.collider.get(), obs.transform);
//         if (v.size() >= 2) {
//             float rad = static_cast<const BaseCollider<Capsule>*>(obs.collider.get())->geometry.radius;
//             float rad_px = rad * GS;
//             float x1 = wx(v[0].x), y1 = wy(v[0].y);
//             float x2 = wx(v[1].x), y2 = wy(v[1].y);
//             float angle = atan2f(y2 - y1, x2 - x1);
//             int segs = (int)(rad_px * 2); if (segs < 24) segs = 24;
//             int n = segs * 2 + 3; // +3: 2 arcs + 1 closing point
//             std::vector<SDL_FPoint> pts(n);
//             // 上半弧: 端1 → 端2 (右侧半圆, 从下到上)
//             for (int i = 0; i <= segs; i++) {
//                 float a = angle + MathX::PI * 0.5f + (float)i / segs * MathX::PI;
//                 pts[i] = { x1 + cosf(a) * rad_px, y1 + sinf(a) * rad_px };
//             }
//             // 下半弧: 端2 → 端1 (左侧半圆, 从上到下)
//             for (int i = 0; i <= segs; i++) {
//                 float a = angle - MathX::PI * 0.5f + (float)i / segs * MathX::PI;
//                 pts[segs + 1 + i] = { x2 + cosf(a) * rad_px, y2 + sinf(a) * rad_px };
//             }
//             // 闭合: 连接回起点 (画底边)
//             pts[n - 1] = pts[0];
//             SDL_SetRenderDrawColor(r, red/2, grn/2, blu/2, 80);
//             SDL_RenderLines(r, pts.data(), n);
//             SDL_SetRenderDrawColor(r, red, grn, blu, 255);
//             SDL_RenderLines(r, pts.data(), n);
//         }
//         break;
//     }
//     case ObsType::Pie: {
//         auto& pie = static_cast<const BaseCollider<Pie>*>(obs.collider.get())->geometry;
//         float sx = wx(obs.transform.position.x), sy = wy(obs.transform.position.y);
//         float sr = pie.radius * GS;
//         float sweep_rad = pie.sweep * MathX::DEG2RAD;
//         float rot_rad = obs.transform.rotation * MathX::DEG2RAD;
//         float cr = cosf(rot_rad), sr_rot = sinf(rot_rad);
//         int segs = (int)(sr * 2); if (segs < 20) segs = 20;
//         std::vector<SDL_FPoint> pts(segs + 3);
//         pts[0] = { sx, sy };
//         int n = 1;
//         for (int i = 0; i <= segs; i++) {
//             float t = -sweep_rad * 0.5f + (float)i / segs * sweep_rad;
//             // 局部坐标: Pie 几何体朝向 +X
//             float lx = cosf(t) * sr, ly = sinf(t) * sr;
//             // 应用旋转 (匹配 collision 的 helper::GetFarthestProjectionPoint)
//             float rx = lx * cr - ly * sr_rot;
//             float ry = lx * sr_rot + ly * cr;
//             pts[n++] = { sx + rx, sy - ry };  // Y 取反转屏幕坐标
//         }
//         pts[n++] = pts[1];
//         SDL_SetRenderDrawColor(r, red/2, grn/2, blu/2, 80);
//         SDL_RenderLines(r, pts.data(), n);
//         SDL_SetRenderDrawColor(r, red, grn, blu, 220);
//         SDL_RenderLines(r, pts.data(), n);
//         break;
//     }
//     case ObsType::Segment: {
//         auto v = get_world_verts(obs.collider.get(), obs.transform);
//         if (v.size() >= 2) {
//             SDL_SetRenderDrawColor(r, red, grn, blu, 220);
//             SDL_RenderLine(r, wx(v[0].x), wy(v[0].y), wx(v[1].x), wy(v[1].y));
//             draw_circle_shape(r, wx(v[0].x), wy(v[0].y), 0.15f, red, grn, blu, 220);
//             draw_circle_shape(r, wx(v[1].x), wy(v[1].y), 0.15f, red, grn, blu, 220);
//         }
//         break;
//     }
//     }
// }

// // ============================================================================
// // 玩家碰撞 / 移动
// // ============================================================================
// // 用 gjk2d C API 检测碰撞
// static bool check_collision_gjk2d(Obstacle& obs, const Vector2& player_pos) {
//     gjk2d_collider_t playerCol;
//     vec2f_t axis = { fp_one(), fp_zero() };
//     gjk2d_init_capsule(&playerCol, toV2f(player_pos), axis,
//         fp_from_float(PLAYER_CAP_LEN), fp_from_float(PLAYER_CAP_RAD));
//     vec2f_t init_dir = vec2f_sub(obs.gjkShape.position, playerCol.position);
//     gjk2d_contact_t c;
//     return gjk2d_collide(&obs.gjkShape, &playerCol, init_dir, &c);
// }

// // ============================================================================
// // SDL 回调
// // ============================================================================
// static SDL_Window* gWin = nullptr;
// static SDL_Renderer* gRen = nullptr;
// static bool gReady = false;

// SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//     run_all_tests();

//     SDL_SetAppMetadata("GJK2D Demo", "1.0", "joy");
//     if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
//     gWin = SDL_CreateWindow("GJK2D Scene Demo (like main8.cpp)", WW, WH, 0);
//     if (!gWin) return SDL_APP_FAILURE;
//     gRen = SDL_CreateRenderer(gWin, NULL);
//     if (!gRen) return SDL_APP_FAILURE;

//     create_scene();
//     gReady = true;
//     SDL_Log("Tests done. Move player with Arrow keys.");
//     return SDL_APP_CONTINUE;
// }

// SDL_AppResult SDL_AppIterate(void* appstate) {
//     if (!gReady) return SDL_APP_CONTINUE;

//     // 玩家移动（类似 main8.cpp 的子步进 + GJK 碰撞检测）
//     const float MOVE_SPEED = 1.5f;
//     const float MAX_STEP = 0.05f;
//     const bool* k = SDL_GetKeyboardState(NULL);
//     Vector2 moveDir = {0, 0};
//     if (k[SDL_SCANCODE_A] || k[SDL_SCANCODE_LEFT])  moveDir.x -= 1;
//     if (k[SDL_SCANCODE_D] || k[SDL_SCANCODE_RIGHT]) moveDir.x += 1;
//     if (k[SDL_SCANCODE_W] || k[SDL_SCANCODE_UP])    moveDir.y += 1;
//     if (k[SDL_SCANCODE_S] || k[SDL_SCANCODE_DOWN])  moveDir.y -= 1;

//     // 只检测玩家与所有障碍物的碰撞（不移动障碍物）
//     bool any_hit = false;
//     for (auto& obs : gObstacles) {
//         if (check_collision_gjk2d(obs, gPlayerPos)) { any_hit = true; break; }
//     }

//     // 子步进移动
//     if (moveDir.x != 0 || moveDir.y != 0) {
//         float len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
//         moveDir.x /= len; moveDir.y /= len;
//         float total = MOVE_SPEED * 0.016f;
//         while (total > 0.0001f) {
//             float step = (total > MAX_STEP) ? MAX_STEP : total;
//             total -= step;
//             Vector2 newPos = { gPlayerPos.x + moveDir.x * step, gPlayerPos.y + moveDir.y * step };

//             bool blocked = false;
//             for (auto& obs : gObstacles) {
//                 gjk2d_collider_t playerCol;
//                 vec2f_t axis = { fp_one(), fp_zero() };
//                 gjk2d_init_capsule(&playerCol, toV2f(newPos), axis,
//                     fp_from_float(PLAYER_CAP_LEN), fp_from_float(PLAYER_CAP_RAD));
//                 vec2f_t init_dir = vec2f_sub(obs.gjkShape.position, playerCol.position);
//                 gjk2d_contact_t c;
//                 if (gjk2d_collide(&obs.gjkShape, &playerCol, init_dir, &c)) {
//                     blocked = true; break;
//                 }
//             }
//             if (!blocked) { gPlayerPos = newPos; any_hit = false; }
//             else break;
//         }
//     }

//     // 边界
//     if (gPlayerPos.x > 12) gPlayerPos.x = 12;
//     if (gPlayerPos.x < -12) gPlayerPos.x = -12;
//     if (gPlayerPos.y > 8) gPlayerPos.y = 8;
//     if (gPlayerPos.y < -8) gPlayerPos.y = -8;

//     // ====== 渲染 ======
//     draw_grid(gRen);

//     // 绘制障碍物
//     for (auto& obs : gObstacles) {
//         bool hit = check_collision_gjk2d(obs, gPlayerPos);
//         draw_obstacle(gRen, obs, hit);
//     }

//     // 绘制玩家（胶囊体）
//     float px = wx(gPlayerPos.x), py = wy(gPlayerPos.y);
//     float hl = PLAYER_CAP_LEN * 0.5f * GS, rad_px = PLAYER_CAP_RAD * GS;
//     float x1 = px - hl, x2 = px + hl;
//     int cap_segs = (int)(rad_px * 2); if (cap_segs < 20) cap_segs = 20;
//     int cap_n = cap_segs * 2 + 3;
//     std::vector<SDL_FPoint> cap_pts(cap_n);
//     for (int i = 0; i <= cap_segs; i++) {
//         float a = 3.14159f * 0.5f + (float)i / cap_segs * 3.14159f;
//         cap_pts[i] = { x1 + cosf(a) * rad_px, py + sinf(a) * rad_px };
//     }
//     for (int i = 0; i <= cap_segs; i++) {
//         float a = -3.14159f * 0.5f + (float)i / cap_segs * 3.14159f;
//         cap_pts[cap_segs + 1 + i] = { x2 + cosf(a) * rad_px, py + sinf(a) * rad_px };
//     }
//     cap_pts[cap_n - 1] = cap_pts[0];
//     SDL_SetRenderDrawColor(gRen, any_hit ? 255 : 100, any_hit ? 80 : 255, 80, 80);
//     SDL_RenderLines(gRen, cap_pts.data(), cap_n);
//     SDL_SetRenderDrawColor(gRen, any_hit ? 255 : 100, any_hit ? 80 : 255, 80, 255);
//     SDL_RenderLines(gRen, cap_pts.data(), cap_n);

//     // 标题信息
//     char ti[256];
//     snprintf(ti, sizeof(ti), "GJK2D Scene - Player(%.1f,%.1f) %s | Arrows=move",
//         gPlayerPos.x, gPlayerPos.y, any_hit ? "HIT!" : "---");
//     SDL_SetWindowTitle(gWin, ti);

//     SDL_RenderPresent(gRen);
//     return SDL_APP_CONTINUE;
// }

// SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//     if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//     if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE)
//         return SDL_APP_SUCCESS;
//     return SDL_APP_CONTINUE;
// }

// void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//     SDL_DestroyRenderer(gRen);
//     SDL_DestroyWindow(gWin);
//     SDL_Quit();
// }
