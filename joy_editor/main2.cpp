//#define SDL_MAIN_USE_CALLBACKS
//#include "SDL3/SDL.h"
//#include "SDL3/SDL_main.h"
//#include <chrono>
//#include <cmath>
//#include <cstring>
//#include <iostream>
//#include <vector>
//#include <cstdio>
//
//// -------------------- 自定义数学库 --------------------
//struct Vec3 {
//    float x, y, z;
//    Vec3() : x(0), y(0), z(0) {}
//    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
//    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
//    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
//    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
//    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
//    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
//    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
//    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
//    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
//};
//
//inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
//inline Vec3 cross(const Vec3& a, const Vec3& b) { return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
//inline float length(const Vec3& v) { return sqrtf(dot(v, v)); }
//inline Vec3 normalize(const Vec3& v) { float len = length(v); return (len > 0) ? v / len : Vec3(0, 0, 0); }
//
//struct Mat4 {
//    float m[16];
//    Mat4() { for (int i = 0; i < 16; ++i) m[i] = 0; m[0] = m[5] = m[10] = m[15] = 1.0f; }
//    Mat4(const float* data) { memcpy(m, data, sizeof(m)); }
//    Mat4 operator*(const Mat4& other) const {
//        Mat4 res;
//        for (int col = 0; col < 4; ++col)
//            for (int row = 0; row < 4; ++row) {
//                float sum = 0;
//                for (int k = 0; k < 4; ++k) sum += m[col * 4 + k] * other.m[k * 4 + row];
//                res.m[col * 4 + row] = sum;
//            }
//        return res;
//    }
//    float& operator()(int col, int row) { return m[col * 4 + row]; }
//    const float& operator()(int col, int row) const { return m[col * 4 + row]; }
//
//    static Mat4 identity() { return Mat4(); }
//    static Mat4 translate(const Vec3& t) {
//        Mat4 mat; mat.m[12] = t.x; mat.m[13] = t.y; mat.m[14] = t.z; return mat;
//    }
//    static Mat4 scale(const Vec3& s) {
//        Mat4 mat; mat.m[0] = s.x; mat.m[5] = s.y; mat.m[10] = s.z; return mat;
//    }
//    static Mat4 rotateX(float angle) {
//        Mat4 mat; float c = cosf(angle), s = sinf(angle);
//        mat.m[5] = c; mat.m[6] = s; mat.m[9] = -s; mat.m[10] = c; return mat;
//    }
//    static Mat4 rotateY(float angle) {
//        Mat4 mat; float c = cosf(angle), s = sinf(angle);
//        mat.m[0] = c; mat.m[2] = -s; mat.m[8] = s; mat.m[10] = c; return mat;
//    }
//    static Mat4 rotateZ(float angle) {
//        Mat4 mat; float c = cosf(angle), s = sinf(angle);
//        mat.m[0] = c; mat.m[1] = s; mat.m[4] = -s; mat.m[5] = c; return mat;
//    }
//    static Mat4 perspective(float fov, float aspect, float near, float far) {
//        float tanHalf = tanf(fov / 2.0f);
//        Mat4 mat; mat.m[0] = 1.0f / (aspect * tanHalf); mat.m[5] = 1.0f / tanHalf;
//        mat.m[10] = far / (near - far); mat.m[11] = -1.0f;
//        mat.m[14] = (near * far) / (near - far); mat.m[15] = 0.0f; return mat;
//    }
//    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
//        Vec3 f = normalize(center - eye); Vec3 s = normalize(cross(f, up)); Vec3 u = cross(s, f);
//        Mat4 mat;
//        mat.m[0] = s.x; mat.m[1] = s.y; mat.m[2] = s.z; mat.m[3] = 0;
//        mat.m[4] = u.x; mat.m[5] = u.y; mat.m[6] = u.z; mat.m[7] = 0;
//        mat.m[8] = -f.x; mat.m[9] = -f.y; mat.m[10] = -f.z; mat.m[11] = 0;
//        mat.m[12] = -dot(s, eye); mat.m[13] = -dot(u, eye); mat.m[14] = dot(f, eye); mat.m[15] = 1.0f;
//        return mat;
//    }
//};
//
//inline float radians(float degrees) { return degrees * (3.14159265358979323846f / 180.0f); }
//
//// -------------------- GPU 资源 --------------------
//struct GPUShaderBundle {
//    SDL_GPUShader* vertex{};
//    SDL_GPUShader* fragment{};
//    operator bool() const { return vertex && fragment; }
//};
//
//SDL_Window* gWindow = nullptr;
//SDL_GPUDevice* gDevice = nullptr;
//GPUShaderBundle gShaders;
//SDL_GPUGraphicsPipeline* gGraphicsPipeline;
//SDL_GPUBuffer* gCubeVertexBuffer;
//SDL_GPUBuffer* gFloorVertexBuffer;
//SDL_GPUBuffer* gUnitCubeVertexBuffer = nullptr;
//SDL_GPUTexture* gTexture;
//SDL_GPUTexture* gTextureEnemy;
//SDL_GPUTexture* gTextureBullet;
//SDL_GPUTexture* gDepthTexture;
//SDL_GPUSampler* gSampler;
//
//#define WINDOW_WIDTH 1024
//#define WINDOW_HEIGHT 720
//
//// 玩家
//Vec3 gPlayerPos = Vec3(0.0f, 0.6f, 0.0f);
//float gPlayerYaw = 0.0f;       // 玩家/相机水平朝向
//float gPitch = 25.0f;          // 相机俯角
//static const float CAM_DIST = 6.0f;
//
//// 时间
//std::chrono::high_resolution_clock::time_point gLastFrameTime;
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
//// MVP 数据
//struct MVPData {
//    Mat4 viewProj;
//    Mat4 model;
//};
//
//struct DrawMesh {
//    SDL_GPUBuffer* vertexBuffer;
//    int vertexCount;
//};
//
//std::vector<DrawMesh> gDrawMeshes;
//std::vector<Mat4> gModelMatrices;
//
//// AABB 碰撞体
//struct AABB {
//    Vec3 min, max;
//};
//
//struct BoxObstacle {
//    Vec3 pos;
//    Vec3 size;
//    AABB aabb;
//};
//
//std::vector<BoxObstacle> gBoxes;
//
//// 斜坡
//struct Ramp {
//    Vec3 basePos;      // 底边中心位置
//    float width;       // X 方向宽度
//    float length;      // Z 方向长度
//    float height;      // Y 方向高度
//
//    // 斜坡沿 +Z 方向上升，从 basePos.y 升到 basePos.y + height
//    // basePos.z - length/2 = 地面端, basePos.z + length/2 = 高端
//    float getHeight(float x, float z) const {
//        float localZ = z - basePos.z;
//        float halfLen = length * 0.5f;
//        float t = (localZ + halfLen) / length;  // 0 = 底端, 1 = 顶端
//        if (t < 0.0f) t = 0.0f;
//        if (t > 1.0f) t = 1.0f;
//        return basePos.y + t * height;
//    }
//
//    bool isInXZ(float x, float z) const {
//        float hw = width * 0.5f;
//        float hl = length * 0.5f;
//        return fabsf(x - basePos.x) <= hw && fabsf(z - basePos.z) <= hl;
//    }
//};
//
//std::vector<Ramp> gRamps;
//
//// 敌人
//struct Enemy {
//    Vec3 pos;
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
//    Vec3 pos;
//    Vec3 dir;
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
//static const Vec3 PLAYER_HALF_SIZE = Vec3(0.3f, 0.6f, 0.3f);
//
//AABB makeAABB(const Vec3& center, const Vec3& halfSize) {
//    return { center - halfSize, center + halfSize };
//}
//
//bool aabbOverlap(const AABB& a, const AABB& b) {
//    return (a.min.x < b.max.x && a.max.x > b.min.x &&
//            a.min.y < b.max.y && a.max.y > b.min.y &&
//            a.min.z < b.max.z && a.max.z > b.min.z);
//}
//
//// -------------------- 初始化 --------------------
//bool initSDL() {
//    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
//        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "SDL init failed");
//        return false;
//    }
//    gWindow = SDL_CreateWindow("First Person Demo", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
//    if (!gWindow) {
//        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL create window failed");
//        return false;
//    }
//
//    gDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
//    if (!gDevice) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "SDL create GPU device failed: %s", SDL_GetError());
//        return false;
//    }
//    if (!SDL_ClaimWindowForGPUDevice(gDevice, gWindow)) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Your system doesn't support SDL GPU");
//        return false;
//    }
//    gLastFrameTime = std::chrono::high_resolution_clock::now();
//    return true;
//}
//
//// -------------------- shader 加载 --------------------
//SDL_GPUShader* loadSDLGPUShader(const char* filename, SDL_GPUShaderStage stage,
//    uint32_t sampler_num, uint32_t uniform_buffer_num) {
//    size_t file_size;
//    Uint8* data = (Uint8*)SDL_LoadFile(filename, &file_size);
//    if (!data) {
//        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "load file %s failed: %s", filename, SDL_GetError());
//        return nullptr;
//    }
//    SDL_GPUShaderCreateInfo ci{};
//    ci.code = data;
//    ci.code_size = file_size;
//    ci.entrypoint = "main";
//    ci.format = SDL_GPU_SHADERFORMAT_SPIRV;
//    ci.num_samplers = sampler_num;
//    ci.num_uniform_buffers = uniform_buffer_num;
//    ci.num_storage_buffers = 0;
//    ci.num_storage_textures = 0;
//    ci.stage = stage;
//    SDL_GPUShader* shader = SDL_CreateGPUShader(gDevice, &ci);
//    SDL_free(data);
//    if (!shader)
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "create shader from %s failed: %s", filename, SDL_GetError());
//    return shader;
//}
//
//GPUShaderBundle createSDLGPUShaderBundle() {
//    GPUShaderBundle bundle;
//    bundle.vertex = loadSDLGPUShader("joy_editor_shaders/vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
//    bundle.fragment = loadSDLGPUShader("joy_editor_shaders/frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
//    return bundle;
//}
//
//// -------------------- 图形管线 --------------------
//SDL_GPUGraphicsPipeline* createGraphicsPipeline() {
//    SDL_GPUGraphicsPipelineCreateInfo ci{};
//    SDL_GPUVertexAttribute attributes[2];
//    attributes[0].location = 0;
//    attributes[0].buffer_slot = 0;
//    attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
//    attributes[0].offset = 0;
//    attributes[1].location = 1;
//    attributes[1].buffer_slot = 0;
//    attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
//    attributes[1].offset = sizeof(float) * 3;
//
//    ci.vertex_input_state.vertex_attributes = attributes;
//    ci.vertex_input_state.num_vertex_attributes = 2;
//
//    SDL_GPUVertexBufferDescription buffer_desc{};
//    buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
//    buffer_desc.instance_step_rate = 0;
//    buffer_desc.slot = 0;
//    buffer_desc.pitch = sizeof(float) * 5;
//
//    ci.vertex_input_state.num_vertex_buffers = 1;
//    ci.vertex_input_state.vertex_buffer_descriptions = &buffer_desc;
//    ci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
//    ci.vertex_shader = gShaders.vertex;
//    ci.fragment_shader = gShaders.fragment;
//    ci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
//    ci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
//    ci.multisample_state.enable_mask = false;
//    ci.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
//    ci.target_info.num_color_targets = 1;
//    ci.target_info.has_depth_stencil_target = true;
//    ci.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
//
//    SDL_GPUDepthStencilState ds{};
//    ds.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
//    ds.back_stencil_state.pass_op = SDL_GPU_STENCILOP_ZERO;
//    ds.back_stencil_state.fail_op = SDL_GPU_STENCILOP_ZERO;
//    ds.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_ZERO;
//    ds.compare_op = SDL_GPU_COMPAREOP_LESS;
//    ds.enable_depth_test = true;
//    ds.enable_depth_write = true;
//    ds.enable_stencil_test = false;
//    ds.compare_mask = 0xFF;
//    ds.write_mask = 0xFF;
//    ci.depth_stencil_state = ds;
//
//    SDL_GPUColorTargetDescription desc{};
//    desc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
//    desc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
//    desc.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_A | SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B;
//    desc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
//    desc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
//    desc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
//    desc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
//    desc.blend_state.enable_blend = true;
//    desc.blend_state.enable_color_write_mask = false;
//    desc.format = SDL_GetGPUSwapchainTextureFormat(gDevice, gWindow);
//    ci.target_info.color_target_descriptions = &desc;
//
//    return SDL_CreateGPUGraphicsPipeline(gDevice, &ci);
//}
//
//// -------------------- 顶点数据 --------------------
//struct Vertex {
//    float x, y, z;
//    float u, v;
//};
//
//SDL_GPUBuffer* createAndUploadBuffer(const Vertex* vertices, int count) {
//    size_t size = count * sizeof(Vertex);
//
//    SDL_GPUTransferBufferCreateInfo tb_ci{};
//    tb_ci.size = size;
//    tb_ci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
//    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(gDevice, &tb_ci);
//    void* ptr = SDL_MapGPUTransferBuffer(gDevice, tb, false);
//    memcpy(ptr, vertices, size);
//    SDL_UnmapGPUTransferBuffer(gDevice, tb);
//
//    SDL_GPUBufferCreateInfo buf_ci{};
//    buf_ci.size = size;
//    buf_ci.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
//    SDL_GPUBuffer* buf = SDL_CreateGPUBuffer(gDevice, &buf_ci);
//
//    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gDevice);
//    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
//    SDL_GPUTransferBufferLocation loc{};
//    loc.offset = 0;
//    loc.transfer_buffer = tb;
//    SDL_GPUBufferRegion region{};
//    region.buffer = buf;
//    region.offset = 0;
//    region.size = size;
//    SDL_UploadToGPUBuffer(copy_pass, &loc, &region, false);
//    SDL_EndGPUCopyPass(copy_pass);
//    SDL_SubmitGPUCommandBuffer(cmd);
//    SDL_ReleaseGPUTransferBuffer(gDevice, tb);
//    return buf;
//}
//
//// 生成地板顶点
//void createFloorMesh() {
//    std::vector<Vertex> verts;
//    float half = 10.0f;
//    int segments = 10;
//    float step = (half * 2) / segments;
//
//    for (int iz = 0; iz < segments; iz++) {
//        for (int ix = 0; ix < segments; ix++) {
//            float x0 = -half + ix * step;
//            float z0 = -half + iz * step;
//            float x1 = x0 + step;
//            float z1 = z0 + step;
//            float u0 = (float)ix / segments;
//            float v0 = (float)iz / segments;
//            float u1 = (float)(ix + 1) / segments;
//            float v1 = (float)(iz + 1) / segments;
//
//            verts.push_back({ x0, 0, z0, u0, v0 });
//            verts.push_back({ x1, 0, z0, u1, v0 });
//            verts.push_back({ x1, 0, z1, u1, v1 });
//
//            verts.push_back({ x0, 0, z0, u0, v0 });
//            verts.push_back({ x1, 0, z1, u1, v1 });
//            verts.push_back({ x0, 0, z1, u0, v1 });
//        }
//    }
//
//    gFloorVertexBuffer = createAndUploadBuffer(verts.data(), (int)verts.size());
//    gDrawMeshes.push_back({ gFloorVertexBuffer, (int)verts.size() });
//    gModelMatrices.push_back(Mat4::identity());
//}
//
//// 生成立方体（玩家用）
//void createCubeMesh() {
//    float hw = 0.3f, hh = 0.6f;
//    Vertex verts[] = {
//        // 前面
//        {-hw, -hh,  hw, 0,0}, { hw, -hh,  hw, 1,0}, { hw,  hh,  hw, 1,1},
//        { hw,  hh,  hw, 1,1}, {-hw,  hh,  hw, 0,1}, {-hw, -hh,  hw, 0,0},
//        // 后面
//        { hw, -hh, -hw, 0,0}, {-hw, -hh, -hw, 1,0}, {-hw,  hh, -hw, 1,1},
//        {-hw,  hh, -hw, 1,1}, { hw,  hh, -hw, 0,1}, { hw, -hh, -hw, 0,0},
//        // 左面
//        {-hw, -hh, -hw, 0,0}, {-hw, -hh,  hw, 1,0}, {-hw,  hh,  hw, 1,1},
//        {-hw,  hh,  hw, 1,1}, {-hw,  hh, -hw, 0,1}, {-hw, -hh, -hw, 0,0},
//        // 右面
//        { hw, -hh,  hw, 0,0}, { hw, -hh, -hw, 1,0}, { hw,  hh, -hw, 1,1},
//        { hw,  hh, -hw, 1,1}, { hw,  hh,  hw, 0,1}, { hw, -hh,  hw, 0,0},
//        // 上面
//        {-hw,  hh,  hw, 0,0}, { hw,  hh,  hw, 1,0}, { hw,  hh, -hw, 1,1},
//        { hw,  hh, -hw, 1,1}, {-hw,  hh, -hw, 0,1}, {-hw,  hh,  hw, 0,0},
//        // 下面
//        {-hw, -hh, -hw, 0,0}, { hw, -hh, -hw, 1,0}, { hw, -hh,  hw, 1,1},
//        { hw, -hh,  hw, 1,1}, {-hw, -hh,  hw, 0,1}, {-hw, -hh, -hw, 0,0},
//    };
//    gCubeVertexBuffer = createAndUploadBuffer(verts, 36);
//}
//
//// 生成立方体（障碍物箱子用）
//void createUnitCubeMesh() {
//    float hw = 0.5f;
//    Vertex verts[] = {
//        // 前面
//        {-hw, -hw,  hw, 0,0}, { hw, -hw,  hw, 1,0}, { hw,  hw,  hw, 1,1},
//        { hw,  hw,  hw, 1,1}, {-hw,  hw,  hw, 0,1}, {-hw, -hw,  hw, 0,0},
//        // 后面
//        { hw, -hw, -hw, 0,0}, {-hw, -hw, -hw, 1,0}, {-hw,  hw, -hw, 1,1},
//        {-hw,  hw, -hw, 1,1}, { hw,  hw, -hw, 0,1}, { hw, -hw, -hw, 0,0},
//        // 左面
//        {-hw, -hw, -hw, 0,0}, {-hw, -hw,  hw, 1,0}, {-hw,  hw,  hw, 1,1},
//        {-hw,  hw,  hw, 1,1}, {-hw,  hw, -hw, 0,1}, {-hw, -hw, -hw, 0,0},
//        // 右面
//        { hw, -hw,  hw, 0,0}, { hw, -hw, -hw, 1,0}, { hw,  hw, -hw, 1,1},
//        { hw,  hw, -hw, 1,1}, { hw,  hw,  hw, 0,1}, { hw, -hw,  hw, 0,0},
//        // 上面
//        {-hw,  hw,  hw, 0,0}, { hw,  hw,  hw, 1,0}, { hw,  hw, -hw, 1,1},
//        { hw,  hw, -hw, 1,1}, {-hw,  hw, -hw, 0,1}, {-hw,  hw,  hw, 0,0},
//        // 下面
//        {-hw, -hw, -hw, 0,0}, { hw, -hw, -hw, 1,0}, { hw, -hw,  hw, 1,1},
//        { hw, -hw,  hw, 1,1}, {-hw, -hw,  hw, 0,1}, {-hw, -hw, -hw, 0,0},
//    };
//    gUnitCubeVertexBuffer = createAndUploadBuffer(verts, 36);
//}
//
//// 生成子弹网格（小立方体）
//SDL_GPUBuffer* gBulletVertexBuffer = nullptr;
//void createBulletMesh() {
//    float hs = 0.1f;
//    Vertex verts[] = {
//        {-hs, -hs,  hs, 0,0}, { hs, -hs,  hs, 1,0}, { hs,  hs,  hs, 1,1},
//        { hs,  hs,  hs, 1,1}, {-hs,  hs,  hs, 0,1}, {-hs, -hs,  hs, 0,0},
//        { hs, -hs, -hs, 0,0}, {-hs, -hs, -hs, 1,0}, {-hs,  hs, -hs, 1,1},
//        {-hs,  hs, -hs, 1,1}, { hs,  hs, -hs, 0,1}, { hs, -hs, -hs, 0,0},
//        {-hs, -hs, -hs, 0,0}, {-hs, -hs,  hs, 1,0}, {-hs,  hs,  hs, 1,1},
//        {-hs,  hs,  hs, 1,1}, {-hs,  hs, -hs, 0,1}, {-hs, -hs, -hs, 0,0},
//        { hs, -hs,  hs, 0,0}, { hs, -hs, -hs, 1,0}, { hs,  hs, -hs, 1,1},
//        { hs,  hs, -hs, 1,1}, { hs,  hs,  hs, 0,1}, { hs, -hs,  hs, 0,0},
//        {-hs,  hs,  hs, 0,0}, { hs,  hs,  hs, 1,0}, { hs,  hs, -hs, 1,1},
//        { hs,  hs, -hs, 1,1}, {-hs,  hs, -hs, 0,1}, {-hs,  hs,  hs, 0,0},
//        {-hs, -hs, -hs, 0,0}, { hs, -hs, -hs, 1,0}, { hs, -hs,  hs, 1,1},
//        { hs, -hs,  hs, 1,1}, {-hs, -hs,  hs, 0,1}, {-hs, -hs, -hs, 0,0},
//    };
//    gBulletVertexBuffer = createAndUploadBuffer(verts, 36);
//}
//
//// 生成斜坡网格
//SDL_GPUBuffer* gRampVertexBuffer = nullptr;
//void createRampMesh() {
//    float w = 2.0f, l = 3.0f, h = 1.5f;
//    float hw = w * 0.5f, hl = l * 0.5f;
//
//    // 斜坡：前低后高，沿 +Z 上升
//    Vertex verts[] = {
//        // 坡面 (前下->前上->后上, 前下->后上->后下)
//        {-hw, 0,  -hl, 0,0}, { hw, 0,  -hl, 1,0}, { hw, h,  hl, 1,1},
//        {-hw, 0,  -hl, 0,0}, { hw, h,  hl, 1,1}, {-hw, h,  hl, 0,1},
//        // 左侧面 (前下->后上->前上, 前下->后下->后上)
//        {-hw, 0,  -hl, 0,0}, {-hw, h,  hl, 0,1}, {-hw, 0,  hl, 1,0},
//        // 右侧面
//        { hw, 0,  -hl, 1,0}, { hw, 0,  hl, 0,1}, { hw, h,  hl, 1,1},
//        // 背面 (高端面, 法线 -Z)
//        {-hw, h,  hl, 0,0}, { hw, h,  hl, 1,0}, { hw, 0,  hl, 1,1},
//        {-hw, h,  hl, 0,0}, { hw, 0,  hl, 1,1}, {-hw, 0,  hl, 0,1},
//        // 底面 (法线 -Y)
//        {-hw, 0, -hl, 0,0}, { hw, 0, -hl, 1,0}, { hw, 0,  hl, 1,1},
//        {-hw, 0, -hl, 0,0}, { hw, 0,  hl, 1,1}, {-hw, 0,  hl, 0,1},
//    };
//    gRampVertexBuffer = createAndUploadBuffer(verts, sizeof(verts)/sizeof(Vertex));
//}
//
//// 创建纹理（科幻金属网格）
//void createFloorTexture() {
//    const int tex_w = 512, tex_h = 512;
//    size_t image_size = tex_w * tex_h * 4;
//    unsigned char* pixels = new unsigned char[image_size];
//    for (int y = 0; y < tex_h; y++) {
//        for (int x = 0; x < tex_w; x++) {
//            int idx = (y * tex_w + x) * 4;
//            bool isGrid = ((x / 32) + (y / 32)) % 2 == 0;
//            if (isGrid) {
//                pixels[idx + 0] = 60;  pixels[idx + 1] = 68;  pixels[idx + 2] = 80;
//            } else {
//                pixels[idx + 0] = 28;  pixels[idx + 1] = 32;  pixels[idx + 2] = 40;
//            }
//            int gx = x % 32, gy = y % 32;
//            if (gx < 1 || gy < 1 || gx > 30 || gy > 30) {
//                pixels[idx + 0] = 30;  pixels[idx + 1] = 180; pixels[idx + 2] = 255;
//            }
//            pixels[idx + 3] = 255;
//        }
//    }
//
//    SDL_GPUTransferBufferCreateInfo tb_ci{};
//    tb_ci.size = image_size;
//    tb_ci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
//    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(gDevice, &tb_ci);
//    void* ptr = SDL_MapGPUTransferBuffer(gDevice, tb, false);
//    memcpy(ptr, pixels, image_size);
//    SDL_UnmapGPUTransferBuffer(gDevice, tb);
//
//    SDL_GPUTextureCreateInfo tex_ci{};
//    tex_ci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
//    tex_ci.height = tex_h;
//    tex_ci.width = tex_w;
//    tex_ci.layer_count_or_depth = 1;
//    tex_ci.num_levels = 1;
//    tex_ci.sample_count = SDL_GPU_SAMPLECOUNT_1;
//    tex_ci.type = SDL_GPU_TEXTURETYPE_2D;
//    tex_ci.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
//    gTexture = SDL_CreateGPUTexture(gDevice, &tex_ci);
//
//    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gDevice);
//    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
//    SDL_GPUTextureTransferInfo ti{};
//    ti.offset = 0;
//    ti.pixels_per_row = tex_w;
//    ti.rows_per_layer = tex_h;
//    ti.transfer_buffer = tb;
//    SDL_GPUTextureRegion region{};
//    region.w = tex_w; region.h = tex_h; region.x = 0; region.y = 0;
//    region.layer = 0; region.mip_level = 0; region.z = 0; region.d = 1;
//    region.texture = gTexture;
//    SDL_UploadToGPUTexture(copy_pass, &ti, &region, false);
//    SDL_EndGPUCopyPass(copy_pass);
//    SDL_SubmitGPUCommandBuffer(cmd);
//    SDL_ReleaseGPUTransferBuffer(gDevice, tb);
//    delete[] pixels;
//}
//
//SDL_GPUTexture* createSolidTexture(unsigned char r, unsigned char g, unsigned char b) {
//    const int tw = 16, th = 16;
//    size_t image_size = tw * th * 4;
//    unsigned char* pixels = new unsigned char[image_size];
//    for (int i = 0; i < tw * th; i++) {
//        pixels[i * 4 + 0] = r;
//        pixels[i * 4 + 1] = g;
//        pixels[i * 4 + 2] = b;
//        pixels[i * 4 + 3] = 255;
//    }
//
//    SDL_GPUTransferBufferCreateInfo tb_ci{};
//    tb_ci.size = image_size;
//    tb_ci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
//    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(gDevice, &tb_ci);
//    void* ptr = SDL_MapGPUTransferBuffer(gDevice, tb, false);
//    memcpy(ptr, pixels, image_size);
//    SDL_UnmapGPUTransferBuffer(gDevice, tb);
//
//    SDL_GPUTextureCreateInfo tex_ci{};
//    tex_ci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
//    tex_ci.height = th;
//    tex_ci.width = tw;
//    tex_ci.layer_count_or_depth = 1;
//    tex_ci.num_levels = 1;
//    tex_ci.sample_count = SDL_GPU_SAMPLECOUNT_1;
//    tex_ci.type = SDL_GPU_TEXTURETYPE_2D;
//    tex_ci.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
//    SDL_GPUTexture* tex = SDL_CreateGPUTexture(gDevice, &tex_ci);
//
//    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gDevice);
//    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
//    SDL_GPUTextureTransferInfo ti{};
//    ti.offset = 0;
//    ti.pixels_per_row = tw;
//    ti.rows_per_layer = th;
//    ti.transfer_buffer = tb;
//    SDL_GPUTextureRegion region{};
//    region.w = tw; region.h = th; region.x = 0; region.y = 0;
//    region.layer = 0; region.mip_level = 0; region.z = 0; region.d = 1;
//    region.texture = tex;
//    SDL_UploadToGPUTexture(copy, &ti, &region, false);
//    SDL_EndGPUCopyPass(copy);
//    SDL_SubmitGPUCommandBuffer(cmd);
//    SDL_ReleaseGPUTransferBuffer(gDevice, tb);
//    delete[] pixels;
//    return tex;
//}
//
//void createEnemyBulletTextures() {
//    gTextureEnemy = createSolidTexture(220, 40, 40);    // 红色敌人
//    gTextureBullet = createSolidTexture(255, 200, 50);  // 黄色子弹
//}
//
//void createDepthTexture(int w, int h) {
//    SDL_GPUTextureCreateInfo tex_ci{};
//    tex_ci.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
//    tex_ci.height = h;
//    tex_ci.width = w;
//    tex_ci.layer_count_or_depth = 1;
//    tex_ci.num_levels = 1;
//    tex_ci.sample_count = SDL_GPU_SAMPLECOUNT_1;
//    tex_ci.type = SDL_GPU_TEXTURETYPE_2D;
//    tex_ci.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
//    gDepthTexture = SDL_CreateGPUTexture(gDevice, &tex_ci);
//}
//
//void createSampler() {
//    SDL_GPUSamplerCreateInfo ci{};
//    ci.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
//    ci.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
//    ci.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
//    ci.enable_anisotropy = true;
//    ci.max_anisotropy = 4.0f;
//    ci.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
//    ci.enable_compare = false;
//    ci.mag_filter = SDL_GPU_FILTER_LINEAR;
//    ci.min_filter = SDL_GPU_FILTER_LINEAR;
//    ci.max_lod = 10.0f;
//    ci.min_lod = 0.0f;
//    ci.mip_lod_bias = 0.0f;
//    ci.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
//    gSampler = SDL_CreateGPUSampler(gDevice, &ci);
//}
//
//MVPData gMVP;
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
//    float yawRad = radians(gPlayerYaw);
//    Vec3 forward = Vec3(sinf(yawRad), 0, cosf(yawRad));
//    Vec3 right   = Vec3(cosf(yawRad), 0, -sinf(yawRad));
//
//    Vec3 moveDir(0,0,0);
//    if (gKeys.forward)  moveDir += forward;
//    if (gKeys.backward) moveDir -= forward;
//    if (gKeys.left)     moveDir += right;
//    if (gKeys.right)    moveDir -= right;
//
//    if (length(moveDir) > 0.001f) {
//        moveDir = normalize(moveDir);
//
//        float totalDist = 5.0f * gDeltaTime;
//        const float MAX_STEP = 0.02f;
//        while (totalDist > 0.0001f) {
//            float step = (totalDist > MAX_STEP) ? MAX_STEP : totalDist;
//            totalDist -= step;
//
//            Vec3 newPos = gPlayerPos + moveDir * step;
//            AABB playerBox = makeAABB(newPos, PLAYER_HALF_SIZE);
//            bool blocked = false;
//            for (auto& box : gBoxes) {
//                if (aabbOverlap(playerBox, box.aabb)) { blocked = true; break; }
//            }
//
//            if (!blocked) {
//                // 空气墙：限制在地板边界内（地板范围 ±10）
//                if (newPos.x > 9.5f) newPos.x = 9.5f;
//                if (newPos.x < -9.5f) newPos.x = -9.5f;
//                if (newPos.z > 9.5f) newPos.z = 9.5f;
//                if (newPos.z < -9.5f) newPos.z = -9.5f;
//                gPlayerPos = newPos;
//            } else {
//                break;
//            }
//        }
//    }
//
//    // 计算地面高度（斜坡、箱子顶部、地面中取最高值）
//    float groundY = 0.0f;  // 默认地面 y = 0
//
//    // 斜坡
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
//        float hw = box.size.x * 0.5f;
//        float hd = box.size.z * 0.5f;
//        if (fabsf(gPlayerPos.x - box.pos.x) < hw + PLAYER_HALF_SIZE.x * 0.8f &&
//            fabsf(gPlayerPos.z - box.pos.z) < hd + PLAYER_HALF_SIZE.z * 0.8f) {
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
//    float yawRad = radians(gPlayerYaw);
//    float pitchRad = radians(gPitch);
//    Vec3 camOffset = Vec3(-sinf(yawRad) * cosf(pitchRad) * CAM_DIST,
//                           sinf(pitchRad) * CAM_DIST,
//                           -cosf(yawRad) * cosf(pitchRad) * CAM_DIST);
//    Vec3 eye = gPlayerPos + camOffset;
//    Vec3 center = gPlayerPos + Vec3(0, 1.0f, 0);
//
//    Mat4 view = Mat4::lookAt(eye, center, Vec3(0,1,0));
//    Mat4 proj = Mat4::perspective(radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
//    Mat4 viewProj = view * proj;
//
//    gMVP.viewProj = viewProj;
//
//    // 地板
//    gModelMatrices[0] = Mat4::identity();
//
//    // 玩家立方体（手动构建矩阵，避免 rotateY 错误）
//    {
//        Mat4 m;
//        float y = radians(gPlayerYaw), c = cosf(y), s = sinf(y);
//        m.m[0] = c;  m.m[2]  = -s;
//        m.m[8] = s;  m.m[10] = c;
//        m.m[12] = gPlayerPos.x;
//        m.m[13] = gPlayerPos.y;
//        m.m[14] = gPlayerPos.z;
//        gModelMatrices[1] = m;
//    }
//
//    // 斜坡（固定位置）
//    gModelMatrices[2] = Mat4::translate(gRamps[0].basePos);
//
//    // 障碍物箱子（斜坡占 index 2，箱子从 3 开始）
//    for (size_t i = 0; i < gBoxes.size(); i++) {
//        auto& box = gBoxes[i];
//        Mat4 m;
//        m.m[0]  = box.size.x;
//        m.m[5]  = box.size.y;
//        m.m[10] = box.size.z;
//        m.m[12] = box.pos.x;
//        m.m[13] = box.pos.y;
//        m.m[14] = box.pos.z;
//        gModelMatrices[3 + i] = m;
//    }
//}
//
//// -------------------- 主循环 --------------------
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//    if (!initSDL()) return SDL_APP_FAILURE;
//
//    gShaders = createSDLGPUShaderBundle();
//    if (!gShaders) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Shader load failed! Program exit!");
//        return SDL_APP_FAILURE;
//    }
//
//    gGraphicsPipeline = createGraphicsPipeline();
//    if (!gGraphicsPipeline) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Graphics pipeline load failed!");
//        return SDL_APP_FAILURE;
//    }
//
//    createFloorMesh();
//
//    createCubeMesh();
//    gModelMatrices.push_back(Mat4::identity());
//    gDrawMeshes.push_back({ gCubeVertexBuffer, 36 });
//
//    createUnitCubeMesh();
//    createRampMesh();
//
//    // 创建斜坡
//    {
//        Ramp ramp;
//        ramp.basePos = Vec3(-6, 0, 0);
//        ramp.width = 2.0f;
//        ramp.length = 3.0f;
//        ramp.height = 1.5f;
//        gRamps.push_back(ramp);
//
//        Mat4 m = Mat4::translate(ramp.basePos);
//        gModelMatrices.push_back(m);
//        gDrawMeshes.push_back({ gRampVertexBuffer, 24 });
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
//            box.pos = Vec3(def.x, def.sy * 0.5f, def.z);
//            box.size = Vec3(def.sx, def.sy, def.sz);
//            box.aabb = makeAABB(box.pos, box.size * 0.5f);
//            gBoxes.push_back(box);
//
//            Mat4 m;
//            m.m[0]  = box.size.x;
//            m.m[5]  = box.size.y;
//            m.m[10] = box.size.z;
//            m.m[12] = box.pos.x;
//            m.m[13] = box.pos.y;
//            m.m[14] = box.pos.z;
//            gModelMatrices.push_back(m);
//            gDrawMeshes.push_back({ gUnitCubeVertexBuffer, 36 });
//        }
//    }
//
//    createFloorTexture();
//    createEnemyBulletTextures();
//    createDepthTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
//    createSampler();
//    createBulletMesh();
//
//    // 创建敌人
//    {
//        struct { float x, z; } enemyPos[] = {
//            { 4, 4 }, { -4, -3 }, { 6, -5 }, { -5, 5 }, { 0, -6 },
//        };
//        for (auto& ep : enemyPos) {
//            Enemy e;
//            e.pos = Vec3(ep.x, 0.6f, ep.z);
//            gEnemies.push_back(e);
//        }
//    }
//
//    gPlayerHP = gPlayerMaxHP;
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    auto now = std::chrono::high_resolution_clock::now();
//    gDeltaTime = std::chrono::duration<float>(now - gLastFrameTime).count();
//    gLastFrameTime = now;
//
//    processInput();
//    updateMVPData();
//
//    // ----- 子弹更新 -----
//    gShootTimer += gDeltaTime;
//    if (gKeys.shoot && gShootTimer >= gShootCooldown) {
//        gShootTimer = 0.0f;
//        float yawRad = radians(gPlayerYaw);
//        Vec3 dir = Vec3(sinf(yawRad), 0, cosf(yawRad));
//        Bullet b;
//        b.pos = gPlayerPos + Vec3(0, 0.4f, 0) + dir * 0.6f;
//        b.dir = dir;
//        gBullets.push_back(b);
//    }
//
//    for (int i = (int)gBullets.size() - 1; i >= 0; i--) {
//        auto& b = gBullets[i];
//        b.age += gDeltaTime;
//        b.pos = b.pos + b.dir * b.speed * gDeltaTime;
//
//        // 子弹超出边界 → 销毁
//        if (fabsf(b.pos.x) > 10.5f || fabsf(b.pos.z) > 10.5f) { gBullets.erase(gBullets.begin() + i); continue; }
//
//        // 子弹击中箱子 → 销毁
//        bool hitBox = false;
//        for (auto& box : gBoxes) {
//            AABB bulBox = makeAABB(b.pos, Vec3(0.15f, 0.15f, 0.15f));
//            if (aabbOverlap(bulBox, box.aabb)) { hitBox = true; break; }
//        }
//        if (hitBox) { gBullets.erase(gBullets.begin() + i); continue; }
//
//        // 子弹击中敌人
//        bool hitEnemy = false;
//        for (auto& e : gEnemies) {
//            if (!e.alive) continue;
//            Vec3 diff = b.pos - e.pos;
//            diff.y = 0;
//            if (length(diff) < 0.6f) {
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
//    // ----- 敌人更新 -----
//    float yawRad = radians(gPlayerYaw);
//    Vec3 playerForward = Vec3(sinf(yawRad), 0, cosf(yawRad));
//    for (auto& e : gEnemies) {
//        if (!e.alive) continue;
//        e.attackTimer += gDeltaTime;
//
//        Vec3 toPlayer = gPlayerPos - e.pos;
//        float dist = length(toPlayer);
//        toPlayer = normalize(toPlayer);
//
//        // 追击玩家
//        if (dist < e.aggroRange) {
//            Vec3 moveDir = toPlayer;
//            moveDir.y = 0;
//            moveDir = normalize(moveDir);
//
//            float totalDist = e.speed * gDeltaTime;
//            const float MAX_STEP = 0.02f;
//            while (totalDist > 0.0001f) {
//                float step = (totalDist > MAX_STEP) ? MAX_STEP : totalDist;
//                totalDist -= step;
//
//                Vec3 newPos = e.pos + moveDir * step;
//                AABB enemyBox = makeAABB(newPos, Vec3(0.25f, 0.25f, 0.25f));
//                bool blocked = false;
//                for (auto& box : gBoxes) {
//                    if (aabbOverlap(enemyBox, box.aabb)) { blocked = true; break; }
//                }
//                if (!blocked) {
//                    e.pos = newPos;
//                } else {
//                    break;
//                }
//            }
//
//            // 空气墙
//            if (e.pos.x > 9.0f) e.pos.x = 9.0f;
//            if (e.pos.x < -9.0f) e.pos.x = -9.0f;
//            if (e.pos.z > 9.0f) e.pos.z = 9.0f;
//            if (e.pos.z < -9.0f) e.pos.z = -9.0f;
//            e.pos.y = 0.6f;
//        }
//
//        // 攻击玩家
//        if (dist < e.attackRange && e.attackTimer >= e.attackCooldown) {
//            e.attackTimer = 0.0f;
//            gPlayerHP -= e.damage;
//            SDL_Log("玩家受到伤害! HP: %d/%d", gPlayerHP, gPlayerMaxHP);
//            if (gPlayerHP <= 0) {
//                SDL_Log("玩家死亡! 重新开始...");
//                gPlayerHP = gPlayerMaxHP;
//                gPlayerPos = Vec3(0, 0.6f, 0);
//            }
//        }
//    }
//
//    bool minimized = SDL_GetWindowFlags(gWindow) & SDL_WINDOW_MINIMIZED;
//    if (minimized) return SDL_APP_CONTINUE;
//
//    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gDevice);
//    SDL_GPUTexture* swapchain = nullptr;
//    Uint32 width, height;
//    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, gWindow, &swapchain, &width, &height)) {
//        return SDL_APP_CONTINUE;
//    }
//    if (!swapchain) return SDL_APP_CONTINUE;
//
//    SDL_GPUColorTargetInfo color{};
//    color.clear_color.r = 0.05f;
//    color.clear_color.g = 0.06f;
//    color.clear_color.b = 0.10f;
//    color.clear_color.a = 1;
//    color.load_op = SDL_GPU_LOADOP_CLEAR;
//    color.mip_level = 0;
//    color.store_op = SDL_GPU_STOREOP_STORE;
//    color.texture = swapchain;
//    color.cycle = true;
//    color.layer_or_depth_plane = 0;
//    color.cycle_resolve_texture = false;
//
//    SDL_GPUDepthStencilTargetInfo depth{};
//    depth.clear_depth = 1;
//    depth.cycle = false;
//    depth.load_op = SDL_GPU_LOADOP_CLEAR;
//    depth.store_op = SDL_GPU_STOREOP_DONT_CARE;
//    depth.texture = gDepthTexture;
//
//    SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &color, 1, &depth);
//    SDL_BindGPUGraphicsPipeline(rp, gGraphicsPipeline);
//
//    int ww, wh;
//    SDL_GetWindowSize(gWindow, &ww, &wh);
//    SDL_GPUViewport vp{};
//    vp.x = 0; vp.y = 0;
//    vp.w = (float)ww; vp.h = (float)wh;
//    vp.min_depth = 0; vp.max_depth = 1;
//    SDL_SetGPUViewport(rp, &vp);
//
//    // 绘制静态物体（地板、箱子等）
//    for (size_t i = 0; i < gDrawMeshes.size(); i++) {
//        auto& mesh = gDrawMeshes[i];
//        gMVP.model = gModelMatrices[i];
//        SDL_PushGPUVertexUniformData(cmd, 0, &gMVP, sizeof(gMVP));
//
//        SDL_GPUTextureSamplerBinding samp{};
//        samp.texture = gTexture;
//        samp.sampler = gSampler;
//        SDL_BindGPUFragmentSamplers(rp, 0, &samp, 1);
//
//        SDL_GPUBufferBinding bind{};
//        bind.buffer = mesh.vertexBuffer;
//        bind.offset = 0;
//        SDL_BindGPUVertexBuffers(rp, 0, &bind, 1);
//
//        SDL_DrawGPUPrimitives(rp, mesh.vertexCount, 1, 0, 0);
//    }
//
//    // 绘制敌人（红色立方体）
//    for (auto& e : gEnemies) {
//        if (!e.alive) continue;
//        Mat4 m;
//        m.m[0] = 0.5f; m.m[5] = 0.5f; m.m[10] = 0.5f;
//        m.m[12] = e.pos.x; m.m[13] = e.pos.y; m.m[14] = e.pos.z;
//        gMVP.model = m;
//        SDL_PushGPUVertexUniformData(cmd, 0, &gMVP, sizeof(gMVP));
//
//        SDL_GPUTextureSamplerBinding samp{};
//        samp.texture = gTextureEnemy;
//        samp.sampler = gSampler;
//        SDL_BindGPUFragmentSamplers(rp, 0, &samp, 1);
//
//        SDL_GPUBufferBinding bind{};
//        bind.buffer = gUnitCubeVertexBuffer;
//        bind.offset = 0;
//        SDL_BindGPUVertexBuffers(rp, 0, &bind, 1);
//        SDL_DrawGPUPrimitives(rp, 36, 1, 0, 0);
//    }
//
//    // 绘制子弹（黄色小立方体）
//    for (auto& b : gBullets) {
//        Mat4 m;
//        m.m[0] = 0.08f; m.m[5] = 0.08f; m.m[10] = 0.08f;
//        m.m[12] = b.pos.x; m.m[13] = b.pos.y; m.m[14] = b.pos.z;
//        gMVP.model = m;
//        SDL_PushGPUVertexUniformData(cmd, 0, &gMVP, sizeof(gMVP));
//
//        SDL_GPUTextureSamplerBinding samp{};
//        samp.texture = gTextureBullet;
//        samp.sampler = gSampler;
//        SDL_BindGPUFragmentSamplers(rp, 0, &samp, 1);
//
//        SDL_GPUBufferBinding bind{};
//        bind.buffer = gBulletVertexBuffer;
//        bind.offset = 0;
//        SDL_BindGPUVertexBuffers(rp, 0, &bind, 1);
//        SDL_DrawGPUPrimitives(rp, 36, 1, 0, 0);
//    }
//
//    SDL_EndGPURenderPass(rp);
//
//    if (!SDL_SubmitGPUCommandBuffer(cmd)) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Submit command buffer failed! %s", SDL_GetError());
//    }
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
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    SDL_WaitForGPUIdle(gDevice);
//    SDL_ReleaseGPUSampler(gDevice, gSampler);
//    SDL_ReleaseGPUTexture(gDevice, gTexture);
//    SDL_ReleaseGPUTexture(gDevice, gTextureEnemy);
//    SDL_ReleaseGPUTexture(gDevice, gTextureBullet);
//    SDL_ReleaseGPUTexture(gDevice, gDepthTexture);
//    SDL_ReleaseGPUBuffer(gDevice, gFloorVertexBuffer);
//    SDL_ReleaseGPUBuffer(gDevice, gCubeVertexBuffer);
//    if (gUnitCubeVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gUnitCubeVertexBuffer);
//    if (gRampVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gRampVertexBuffer);
//    if (gBulletVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gBulletVertexBuffer);
//    SDL_ReleaseGPUGraphicsPipeline(gDevice, gGraphicsPipeline);
//    SDL_ReleaseGPUShader(gDevice, gShaders.vertex);
//    SDL_ReleaseGPUShader(gDevice, gShaders.fragment);
//    SDL_ReleaseWindowFromGPUDevice(gDevice, gWindow);
//    SDL_DestroyGPUDevice(gDevice);
//    SDL_DestroyWindow(gWindow);
//    SDL_Quit();
//}
