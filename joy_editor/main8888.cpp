//#define SDL_MAIN_USE_CALLBACKS
//#include "SDL3/SDL.h"
//#include "SDL3/SDL_main.h"
//#include <joy/joy.h>
//#include <cmath>
//#include <cstring>
//#include <cstdio>
//#include <vector>
//
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
//
//#define WINDOW_WIDTH 1024
//#define WINDOW_HEIGHT 720
//
//// 玩家
//vec3_t gPlayerPos = {0.0f, 0.6f, 0.0f};
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
//
//
//// -------------------- 初始化 --------------------
//bool initSDL() {
//    gWin = create_window("First Person Demo", WINDOW_WIDTH, WINDOW_HEIGHT);
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
//    gPlayerMesh = r3d_mesh_cube_lit(gR3d, 0.6f, 1.2f, 0.6f);
//    gUnitCubeMesh = r3d_mesh_cube_lit(gR3d, 1.0f, 1.0f, 1.0f);
//    gSphereMesh = r3d_mesh_sphere_lit(gR3d, 0.5f, 16, 12);
//    gBulletMesh = r3d_mesh_cube_lit(gR3d, 0.16f, 0.16f, 0.16f);
//    gRampMesh = r3d_mesh_ramp_lit(gR3d, 2.0f, 3.0f, 1.5f);
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
//    // 玩家立方体（手动构建矩阵，避免 rotateY 错误）
//    {
//        mat44_t m = mat44_identity();
//        float y = ft_radians(gPlayerYaw), c = cosf(y), s = sinf(y);
//        m.m0 = c;  m.m2  = -s;
//        m.m8 = s;  m.m10 = c;
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
//    // 玩家
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
//    /* 敌人已屏蔽
//    {
//        struct { float x, z; } enemyPos[] = {
//            { 4, 4 }, { -4, -3 }, { 6, -5 }, { -5, 5 }, { 0, -6 },
//        };
//        for (auto& ep : enemyPos) {
//            Enemy e;
//            e.pos = {ep.x, 0.6f, ep.z};
//            gEnemies.push_back(e);
//        }
//    }*/
//
//    gPlayerHP = gPlayerMaxHP;
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
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
//    /* 敌人已屏蔽
//    float yawRad = ft_radians(gPlayerYaw);
//    vec3_t playerForward = {sinf(yawRad), 0, cosf(yawRad)};
//    for (auto& e : gEnemies) { }
//    */
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
//        r3d_mesh_destroy(gR3d, gPlayerMesh);
//        r3d_mesh_destroy(gR3d, gUnitCubeMesh);
//        r3d_mesh_destroy(gR3d, gSphereMesh);
//        r3d_mesh_destroy(gR3d, gBulletMesh);
//        r3d_mesh_destroy(gR3d, gRampMesh);
//        r3d_destroy(gR3d);
//        gR3d = nullptr;
//    }
//    if (gWin) {
//        destroy_window(gWin);
//        gWin = nullptr;
//    }
//}
