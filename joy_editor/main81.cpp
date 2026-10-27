// #define SDL_MAIN_USE_CALLBACKS
// #include "SDL3/SDL.h"
// #include "SDL3/SDL_main.h"
// #include <chrono>
// #include <cmath>
// #include <cstring>
// #include <iostream>
//#include <vector>
//#include <string>
//#include <cstdio>
//#include <algorithm>
//#include <map>
// #include <joy/joy.h>
// #include <cfloat>
// #include "joy/external/cgltf.h"
// #include "joy/external/stb_image.h"
//
//
// static vec3f_t toV3f(vec3_t v) { return { fp_from_float(v.x), fp_from_float(v.y), fp_from_float(v.z) }; }
//
// // -------------------- GPU 资源 --------------------
// struct GPUShaderBundle {
//    SDL_GPUShader* vertex{};
//    SDL_GPUShader* fragment{};
//    operator bool() const { return vertex && fragment; }
// };
//
// SDL_Window* gWindow = nullptr;
// SDL_GPUDevice* gDevice = nullptr;
// GPUShaderBundle gShaders;
// SDL_GPUGraphicsPipeline* gGraphicsPipeline;
// SDL_GPUBuffer* gCubeVertexBuffer;
// SDL_GPUBuffer* gFloorVertexBuffer;
// SDL_GPUBuffer* gUnitCubeVertexBuffer = nullptr;
// SDL_GPUTexture* gTexture;
// SDL_GPUTexture* gTextureEnemy;
// SDL_GPUTexture* gTextureBullet;
// SDL_GPUTexture* gDepthTexture;
// SDL_GPUSampler* gSampler;
//
// #define WINDOW_WIDTH 1024
// #define WINDOW_HEIGHT 720
//
// // 玩家
// vec3_t gPlayerPos = {0.0f, 0.6f, 0.0f};
// float gPlayerYaw = 0.0f;       // 玩家/相机水平朝向
// float gPitch = 25.0f;          // 相机俯角
// static const float CAM_DIST = 6.0f;
//
// // 时间
// std::chrono::high_resolution_clock::time_point gLastFrameTime;
// float gDeltaTime = 0.0f;
//
// // 输入
// struct {
//    bool forward = false;
//    bool backward = false;
//    bool left = false;
//    bool right = false;
//    bool jump = false;
//    bool shoot = false;
// } gKeys;
//
// float gVerticalVelocity = 0.0f;
// bool gOnGround = true;
//
// render_mvp_t gMVP;
//
// struct DrawMesh {
//    SDL_GPUBuffer* vertexBuffer;
//    int vertexCount;
// };
//
// std::vector<DrawMesh> gDrawMeshes;
// std::vector<mat44_t> gModelMatrices;
//
// struct BoxObstacle {
//    vec3_t pos;
//    vec3_t size;
//    gjk3df_collider_t gjkShape;
//    BoxObstacle() { memset(&gjkShape, 0, sizeof(gjkShape)); }
// };
//
// std::vector<BoxObstacle> gBoxes;
//
// // GJK 测试球体
// struct BallObstacle {
//    vec3_t pos;
//    float radius;
//    gjk3df_collider_t gjkShape;
//    BallObstacle() { memset(&gjkShape, 0, sizeof(gjkShape)); }
// };
// std::vector<BallObstacle> gBalls;
// SDL_GPUTexture* gTextureBall = nullptr;
//
//// glTF 模型
//SDL_GPUBuffer* gGLTFVertexBuffer = nullptr;
//int gGLTFVertexCount = 0;
//SDL_GPUTexture* gGLTFTexture = nullptr;
//bool gGLTFValid = false;
//cgltf_data* gGLTFData = nullptr;
//float gGLTFAnimTime = 0.0f;
//bool gGLTFAnimPlaying = false;
//int gGLTFAnimIndex = 0;
//
//// 顶点格式（move forward for global refs）
//struct Vertex { float x, y, z; float u, v; };
//
//// 逐网格部件信息（用于动画）
//struct MeshPart {
//    int vertexOffset;
//    int vertexCount;
//    cgltf_node* node; // 该网格所属的节点（用于计算世界矩阵）
//};
//std::vector<MeshPart> gMeshParts;
//std::vector<Vertex> gRestPoseVerts; // 原始顶点（rest pose）
//
// // 旋转箱子（OBB，GJK 支持但 AABB 不支持）
// struct OBBObstacle {
//    vec3_t pos;
//    vec3_t size;
//    float angle;
//    gjk3df_collider_t gjkShape;
//    vec3f_t worldVerts[8];
// };
// std::vector<OBBObstacle> gOBBs;
// SDL_GPUTexture* gTextureOBB = nullptr;
//
// // 斜坡（长方体斜放作为斜坡）
// struct RampObstacle {
//    vec3_t basePos;
//    float width, length, height;
//    float angle;  // 绕X轴旋转角度
//    gjk3df_collider_t gjkShape;
//    vec3f_t worldVerts[8];
//
//    float getHeight(float x, float z) const {
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
//        float cs = cosf(angle), sn = sinf(angle);
//        float hw = width * 0.5f, hl = length * 0.5f, hh = height * 0.5f;
//        float x_local = x - basePos.x;
//        float z_local = (z - basePos.z - hh * sn) / cs;
//        return fabsf(x_local) <= hw + 0.2f && fabsf(z_local) <= hl + 0.2f;
//    }
// };
//
// std::vector<RampObstacle> gRamps;
//
// // 敌人
// struct Enemy {
//    vec3_t pos;
//    float hp = 3.0f;
//    float speed = 2.0f;
//    float aggroRange = 8.0f;      // 发现玩家距离
//    float attackRange = 1.2f;     // 攻击距离
//    float attackCooldown = 1.0f;
//    float attackTimer = 0.0f;
//    int damage = 1;
//    bool alive = true;
// };
//
// std::vector<Enemy> gEnemies;
// int gPlayerHP = 10;
// int gPlayerMaxHP = 10;
//
// // 子弹
// struct Bullet {
//    vec3_t pos;
//    vec3_t dir;
//    float speed = 15.0f;
//    float lifetime = 2.0f;
//    float age = 0.0f;
//    int damage = 1;
//    bool active = true;
// };
//
// std::vector<Bullet> gBullets;
// float gShootCooldown = 0.25f;
// float gShootTimer = 0.0f;
//
// static const vec3_t PLAYER_HALF_SIZE = {0.3f, 0.6f, 0.3f};
//
//
//
// // -------------------- 初始化 --------------------
// bool initSDL() {
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
// }
//
// GPUShaderBundle createSDLGPUShaderBundle() {
//    GPUShaderBundle bundle;
//     bundle.vertex = render_load_shader(gDevice, "joy_editor_shaders/vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
//     bundle.fragment = render_load_shader(gDevice, "joy_editor_shaders/frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
//    return bundle;
// }
//
//// 生成地板顶点
// void createFloorMesh() {
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
//    gFloorVertexBuffer = render_upload_buffer(gDevice, verts.data(), verts.size() * sizeof(Vertex));
//    gDrawMeshes.push_back({ gFloorVertexBuffer, (int)verts.size() });
//    gModelMatrices.push_back(mat44_identity());
// }
//
// // 生成立方体（玩家用）
// void createCubeMesh() {
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
//    gCubeVertexBuffer = render_upload_buffer(gDevice, verts, sizeof(verts));
// }
//
// // 生成立方体（障碍物箱子用）
// void createUnitCubeMesh() {
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
//    gUnitCubeVertexBuffer = render_upload_buffer(gDevice, verts, sizeof(verts));
// }
//
// // 生成子弹网格（小立方体）
// SDL_GPUBuffer* gBulletVertexBuffer = nullptr;
// SDL_GPUBuffer* gSphereVertexBuffer = nullptr;
//
// // 生成球体网格（UV 球体）
// void createSphereMesh() {
//    std::vector<Vertex> verts;
//    const int slices = 16, stacks = 12;
//    for (int j = 0; j < stacks; j++) {
//        float phi1 = (float)j / stacks * 3.14159265f;
//        float phi2 = (float)(j + 1) / stacks * 3.14159265f;
//        for (int i = 0; i < slices; i++) {
//            float theta1 = (float)i / slices * 2.0f * 3.14159265f;
//            float theta2 = (float)(i + 1) / slices * 2.0f * 3.14159265f;
//
//            float x1 = sinf(phi1) * cosf(theta1), y1 = cosf(phi1), z1 = sinf(phi1) * sinf(theta1);
//            float x2 = sinf(phi1) * cosf(theta2), y2 = cosf(phi1), z2 = sinf(phi1) * sinf(theta2);
//            float x3 = sinf(phi2) * cosf(theta2), y3 = cosf(phi2), z3 = sinf(phi2) * sinf(theta2);
//            float x4 = sinf(phi2) * cosf(theta1), y4 = cosf(phi2), z4 = sinf(phi2) * sinf(theta1);
//
//            float u1 = (float)i / slices, u2 = (float)(i + 1) / slices;
//            float v1 = (float)j / stacks, v2 = (float)(j + 1) / stacks;
//
//            verts.push_back({ x1, y1, z1, u1, v1 });
//            verts.push_back({ x2, y2, z2, u2, v1 });
//            verts.push_back({ x3, y3, z3, u2, v2 });
//            verts.push_back({ x1, y1, z1, u1, v1 });
//            verts.push_back({ x3, y3, z3, u2, v2 });
//            verts.push_back({ x4, y4, z4, u1, v2 });
//        }
//    }
//    gSphereVertexBuffer = render_upload_buffer(gDevice, verts.data(), verts.size() * sizeof(Vertex));
// }
//
// void createBulletMesh() {
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
//    gBulletVertexBuffer = render_upload_buffer(gDevice, verts, sizeof(verts));
// }
//
//// 提取目录路径（包含末尾分隔符）
//static std::string getDir(const char* path) {
//    std::string s(path);
//    auto pos = s.find_last_of("/\\");
//    return (pos != std::string::npos) ? s.substr(0, pos + 1) : "";
//}
//
//// 加载 glTF 模型并上传到 GPU
//static bool loadGLTFModel(const char* path, SDL_GPUBuffer** outBuf, int* outCount) {
//    *outBuf = nullptr;
//    *outCount = 0;
//    gGLTFTexture = nullptr;
//    gGLTFValid = false;
//    gGLTFData = nullptr;
//
//    cgltf_options opts = {};
//        gMeshParts.clear();
//    gRestPoseVerts.clear();
//    cgltf_result res = cgltf_parse_file(&opts, path, &gGLTFData);
//    if (res != cgltf_result_success) {
//        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "cgltf_parse_file failed: %s", path);
//        return false;
//    }
//
//    res = cgltf_load_buffers(&opts, gGLTFData, path);
//    if (res != cgltf_result_success) {
//        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "cgltf_load_buffers failed: %s", path);
//        cgltf_free(gGLTFData); gGLTFData = nullptr;
//        return false;
//    }
//
//    std::string baseDir = getDir(path);
//
//    // ---- 加载纹理：直接枚举所有图片 ----
//    auto loadTexture = [&](cgltf_image* img, int idx) -> bool {
//        if (!img) { SDL_Log("image[%d]: null", idx); return false; }
//        const uint8_t* imgData = nullptr;
//        cgltf_size imgSize = 0;
//        int w, h, n;
//        unsigned char* pixels = nullptr;
//
//        SDL_Log("image[%d]: name=%s buffer_view=%p uri=%s", idx,
//            img->name ? img->name : "(null)",
//            (void*)img->buffer_view,
//            img->uri ? img->uri : "(null)");
//
//        if (img->buffer_view && img->buffer_view->buffer && img->buffer_view->buffer->data) {
//            imgData = (const uint8_t*)img->buffer_view->buffer->data + img->buffer_view->offset;
//            imgSize = img->buffer_view->size;
//            SDL_Log("  -> embedded %zu bytes", imgSize);
//            pixels = stbi_load_from_memory(imgData, (int)imgSize, &w, &h, &n, 4);
//        } else if (img->uri) {
//            std::string texPath = baseDir + img->uri;
//            SDL_Log("  -> external %s", texPath.c_str());
//            pixels = stbi_load(texPath.c_str(), &w, &h, &n, 4);
//        } else {
//            SDL_Log("  -> no data source");
//        }
//        if (!pixels) { SDL_Log("  -> stbi_load FAILED"); return false; }
//
//        SDL_Log("  -> decoded %dx%d (%d channels)", w, h, n);
//        SDL_GPUTextureCreateInfo tci{};
//        tci.type = SDL_GPU_TEXTURETYPE_2D;
//        tci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
//        tci.width = w; tci.height = h;
//        tci.layer_count_or_depth = 1;
//        tci.num_levels = 1;
//        tci.sample_count = SDL_GPU_SAMPLECOUNT_1;
//        tci.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
//        gGLTFTexture = SDL_CreateGPUTexture(gDevice, &tci);
//        if (!gGLTFTexture) { stbi_image_free(pixels); SDL_Log("  -> CreateGPUTexture FAILED"); return false; }
//
//        SDL_GPUTransferBufferCreateInfo tbci{};
//        tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
//        tbci.size = (Uint32)(w * h * 4);
//        SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(gDevice, &tbci);
//        memcpy(SDL_MapGPUTransferBuffer(gDevice, tb, false), pixels, w * h * 4);
//        SDL_UnmapGPUTransferBuffer(gDevice, tb);
//        SDL_GPUCommandBuffer* cb = SDL_AcquireGPUCommandBuffer(gDevice);
//        SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cb);
//        SDL_GPUTextureTransferInfo src{};
//        src.transfer_buffer = tb; src.offset = 0;
//        src.pixels_per_row = w; src.rows_per_layer = h;
//        SDL_GPUTextureRegion dst{};
//        dst.texture = gGLTFTexture; dst.w = w; dst.h = h; dst.d = 1;
//        SDL_UploadToGPUTexture(cp, &src, &dst, false);
//        SDL_EndGPUCopyPass(cp);
//        SDL_SubmitGPUCommandBuffer(cb);
//        SDL_ReleaseGPUTransferBuffer(gDevice, tb);
//        stbi_image_free(pixels);
//        SDL_Log("  -> texture loaded OK");
//        return true;
//    };
//
//    SDL_Log("GLTF: %zu images, %zu textures, %zu materials",
//        gGLTFData->images_count, gGLTFData->textures_count, gGLTFData->materials_count);
//    for (cgltf_size i = 0; i < gGLTFData->images_count && !gGLTFTexture; i++) {
//        loadTexture(&gGLTFData->images[i], (int)i);
//    }
//    if (!gGLTFTexture)
//        SDL_Log("WARNING: no texture loaded (model may use vertex colors or solid materials)");
//
//    // ---- 建立 mesh → node 映射 ----
//    std::map<cgltf_mesh*, cgltf_node*> meshToNode;
//    for (cgltf_size n = 0; n < gGLTFData->nodes_count; n++) {
//        cgltf_node* node = &gGLTFData->nodes[n];
//        if (node->mesh)
//            meshToNode[node->mesh] = node;
//    }
//
//    // ---- 收集顶点（逐 mesh 存储）----
//    std::vector<Vertex> verts;
//    gMeshParts.clear();
//    gRestPoseVerts.clear();
//    cgltf_data* data = gGLTFData;
//
//    for (cgltf_size m = 0; m < data->meshes_count; m++) {
//        cgltf_mesh* mesh = &data->meshes[m];
//        cgltf_node* node = meshToNode[mesh]; // 可能为 NULL
//
//        for (cgltf_size p = 0; p < mesh->primitives_count; p++) {
//            cgltf_primitive* prim = &mesh->primitives[p];
//            const float* posData = nullptr;
//            cgltf_size posCount = 0;
//            for (cgltf_size a = 0; a < prim->attributes_count; a++) {
//                cgltf_attribute* at = &prim->attributes[a];
//                if (at->type == cgltf_attribute_type_position) {
//                    cgltf_accessor* acc = at->data;
//                    if (acc->buffer_view && acc->buffer_view->buffer && acc->buffer_view->buffer->data) {
//                        posData = (const float*)((const uint8_t*)acc->buffer_view->buffer->data + acc->offset + acc->buffer_view->offset);
//                        posCount = acc->count;
//                    }
//                    break;
//                }
//            }
//            if (!posData || posCount == 0) continue;
//
//            const float* uvData = nullptr;
//            for (cgltf_size a = 0; a < prim->attributes_count; a++) {
//                cgltf_attribute* at = &prim->attributes[a];
//                if (at->type == cgltf_attribute_type_texcoord) {
//                    cgltf_accessor* acc = at->data;
//                    if (acc->buffer_view && acc->buffer_view->buffer && acc->buffer_view->buffer->data) {
//                        uvData = (const float*)((const uint8_t*)acc->buffer_view->buffer->data + acc->offset + acc->buffer_view->offset);
//                    }
//                    break;
//                }
//            }
//
//            auto readIdx = [](cgltf_accessor* acc, cgltf_size i) -> uint32_t {
//                const uint8_t* base = (const uint8_t*)acc->buffer_view->buffer->data + acc->offset + acc->buffer_view->offset;
//                if (acc->component_type == cgltf_component_type_r_16u) return ((uint16_t*)base)[i];
//                if (acc->component_type == cgltf_component_type_r_32u) return ((uint32_t*)base)[i];
//                return ((uint8_t*)base)[i];
//            };
//
//            int partStart = (int)verts.size();
//            if (prim->indices) {
//                cgltf_accessor* idxAcc = prim->indices;
//                for (cgltf_size i = 0; i < idxAcc->count; i++) {
//                    uint32_t idx = readIdx(idxAcc, i);
//                    if (idx >= posCount) continue;
//                    float x = posData[idx * 3 + 0];
//                    float y = posData[idx * 3 + 1];
//                    float z = posData[idx * 3 + 2];
//                    float u = uvData ? uvData[idx * 2 + 0] : 0.5f;
//                    float v = uvData ? uvData[idx * 2 + 1] : 0.5f;
//                    verts.push_back({x, y, z, u, v});
//                }
//            } else {
//                for (cgltf_size i = 0; i < posCount; i++) {
//                    float x = posData[i * 3 + 0];
//                    float y = posData[i * 3 + 1];
//                    float z = posData[i * 3 + 2];
//                    float u = uvData ? uvData[i * 2 + 0] : 0.5f;
//                    float v = uvData ? uvData[i * 2 + 1] : 0.5f;
//                    verts.push_back({x, y, z, u, v});
//                }
//            }
//            int partCount = (int)verts.size() - partStart;
//            if (partCount > 0)
//                gMeshParts.push_back({ partStart, partCount, node });
//        }
//    }
//
//    if (verts.empty()) {
//        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "No vertices found in %s", path);
//        cgltf_free(gGLTFData); gGLTFData = nullptr;
//        return false;
//    }
//
//    // 保存 rest pose
//    gRestPoseVerts = verts;
//
//    *outBuf = render_upload_buffer(gDevice, verts.data(), verts.size() * sizeof(Vertex));
//    *outCount = (int)verts.size();
//    gGLTFVertexBuffer = *outBuf;
//    gGLTFValid = *outBuf != nullptr;
//    SDL_Log("GLTF loaded: %s - %zu vertices in %zu parts, texture=%s, animations=%zu",
//        path, verts.size(), gMeshParts.size(), gGLTFTexture ? "YES" : "NO", data->animations_count);
//    return gGLTFValid;
//}
//
// // 生成斜坡网格
// SDL_GPUBuffer* gRampVertexBuffer = nullptr;
// void createRampMesh() {
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
//    gRampVertexBuffer = render_upload_buffer(gDevice, verts, sizeof(verts));
// }
//
// // 创建纹理（科幻金属网格）
// void createFloorTexture() {
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
// }
//
// void createEnemyBulletTextures() {
//    gTextureEnemy = render_create_solid_texture(gDevice, 220, 40, 40);    // 红色敌人
//    gTextureBullet = render_create_solid_texture(gDevice, 255, 200, 50);  // 黄色子弹
//    gTextureBall = render_create_solid_texture(gDevice, 40, 220, 40);     // 绿色球体
//    gTextureOBB = render_create_solid_texture(gDevice, 255, 165, 0);      // 橙色旋转箱子
// }
//
// // -------------------- 输入处理 --------------------
// void processInput() {
//    const bool* kb = SDL_GetKeyboardState(NULL);
//    gKeys.forward  = kb[SDL_SCANCODE_W];
//    gKeys.backward = kb[SDL_SCANCODE_S];
//    gKeys.left     = kb[SDL_SCANCODE_A];
//    gKeys.right    = kb[SDL_SCANCODE_D];
//    gKeys.jump     = kb[SDL_SCANCODE_SPACE];
//    gKeys.shoot    = kb[SDL_SCANCODE_F];
//
//    float yawRad = ft_radians(gPlayerYaw);
//    vec3_t forward = {sinf(yawRad), 0, cosf(yawRad)};
//    vec3_t right   = {cosf(yawRad), 0, -sinf(yawRad)};
//
//    vec3_t moveDir = {0,0,0};
//    if (gKeys.forward)  moveDir = vec3_add(moveDir, forward);
//    if (gKeys.backward) moveDir = vec3_sub(moveDir, forward);
//    if (gKeys.left)     moveDir = vec3_add(moveDir, right);
//    if (gKeys.right)    moveDir = vec3_sub(moveDir, right);
//
//    if (vec3_length(moveDir) > 0.001f) {
//        moveDir = vec3_normalize(moveDir);
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
//            gjk3df_collider_t playerShape;
//            gjk3df_init_box(&playerShape, toV3f(newPos), toV3f(PLAYER_HALF_SIZE));
//            vec3f_t gjkInitDir = { fp_one(), fp_zero(), fp_zero() };
//            bool blocked = false;
//            for (auto& box : gBoxes) {
//                // 玩家站在箱子顶部时跳过碰撞（仅当脚底几乎贴住箱顶表面）
//                float boxTop = box.pos.y + box.size.y * 0.5f;
//                float playerBottom = newPos.y - PLAYER_HALF_SIZE.y;
//                bool onBox = playerBottom >= boxTop - 0.01f && playerBottom <= boxTop + 0.05f &&
//                    fabsf(newPos.x - box.pos.x) <= box.size.x * 0.5f + PLAYER_HALF_SIZE.x * 0.5f &&
//                    fabsf(newPos.z - box.pos.z) <= box.size.z * 0.5f + PLAYER_HALF_SIZE.z * 0.5f;
//                // 往边缘走时 newPos 可能略微超出, 但当前位置还在箱子上
//                if (!onBox) {
//                    float curBottom = gPlayerPos.y - PLAYER_HALF_SIZE.y;
//                    onBox = curBottom >= boxTop - 0.01f && curBottom <= boxTop + 0.05f &&
//                        fabsf(gPlayerPos.x - box.pos.x) <= box.size.x * 0.5f + PLAYER_HALF_SIZE.x * 0.5f &&
//                        fabsf(gPlayerPos.z - box.pos.z) <= box.size.z * 0.5f + PLAYER_HALF_SIZE.z * 0.5f;
//                }
//                if (onBox) continue;
//                gjk3df_contact_t c;
//                if (gjk3df_collide(&playerShape, &box.gjkShape, gjkInitDir, &c)) { blocked = true; break; }
//            }
//            if (!blocked) {
//                for (auto& ball : gBalls) {
//                    gjk3df_contact_t c;
//                    if (gjk3df_collide(&playerShape, &ball.gjkShape, gjkInitDir, &c)) { blocked = true; break; }
//                }
//            }
//            if (!blocked) {
//                for (auto& obb : gOBBs) {
//                    gjk3df_contact_t c;
//                    if (gjk3df_collide(&playerShape, &obb.gjkShape, gjkInitDir, &c)) { blocked = true; break; }
//                }
//            }
//            if (!blocked) {
//                for (auto& ramp : gRamps) {
//                    // 玩家站在斜坡上时跳过碰撞
//                    bool onRamp = ramp.isInXZ(newPos.x, newPos.z) &&
//                        fabsf(newPos.y - PLAYER_HALF_SIZE.y - ramp.getHeight(newPos.x, newPos.z)) < 0.3f;
//                    // 从斜坡顶往边缘走时, isInXZ(newPos) 会 false, 但当前位置还在坡上
//                    if (!onRamp) {
//                        onRamp = ramp.isInXZ(gPlayerPos.x, gPlayerPos.z) &&
//                            fabsf(gPlayerPos.y - PLAYER_HALF_SIZE.y - ramp.getHeight(gPlayerPos.x, gPlayerPos.z)) < 0.3f;
//                    }
//                    if (onRamp) continue;
//                    gjk3df_contact_t c;
//                    if (gjk3df_collide(&playerShape, &ramp.gjkShape, gjkInitDir, &c)) { blocked = true; break; }
//                }
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
// }
//
// // -------------------- 更新 MVP --------------------
// void updateMVPData() {
//    float yawRad = ft_radians(gPlayerYaw);
//    float pitchRad = ft_radians(gPitch);
//    vec3_t camOffset = {-sinf(yawRad) * cosf(pitchRad) * CAM_DIST,
//                           sinf(pitchRad) * CAM_DIST,
//                           -cosf(yawRad) * cosf(pitchRad) * CAM_DIST};
//    vec3_t eye = vec3_add(gPlayerPos, camOffset);
//    vec3_t center = vec3_add(gPlayerPos, {0, 1.0f, 0});
//
//    mat44_t view = mat44_lookAt(eye, center, {0,1,0});
//    mat44_t proj = mat44_perspective(ft_radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
//    mat44_t viewProj = mat44_mul(view, proj);
//
//    render_mat44_to_float16(&viewProj, gMVP.viewProj);
//
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
// }
//
// // -------------------- 主循环 --------------------
// SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//    if (!initSDL()) return SDL_APP_FAILURE;
//
//    gShaders = createSDLGPUShaderBundle();
//    if (!gShaders) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Shader load failed! Program exit!");
//        return SDL_APP_FAILURE;
//    }
//
//    gGraphicsPipeline = render_create_pipeline(gDevice, gShaders.vertex, gShaders.fragment, SDL_GetGPUSwapchainTextureFormat(gDevice, gWindow), SDL_GPU_TEXTUREFORMAT_D16_UNORM);
//    if (!gGraphicsPipeline) {
//        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Graphics pipeline load failed!");
//        return SDL_APP_FAILURE;
//    }
//
//    createFloorMesh();
//
//    createCubeMesh();
//    gModelMatrices.push_back(mat44_identity());
//    gDrawMeshes.push_back({ gCubeVertexBuffer, 36 });
//
//    createUnitCubeMesh();
//    createRampMesh();
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
//        {
//            ramp.worldVerts[0] = toV3f({ramp.basePos.x - ramp.width * 0.5f, ramp.basePos.y, ramp.basePos.z - ramp.length * 0.5f});
//            ramp.worldVerts[1] = toV3f({ramp.basePos.x + ramp.width * 0.5f, ramp.basePos.y, ramp.basePos.z - ramp.length * 0.5f});
//            ramp.worldVerts[2] = toV3f({ramp.basePos.x - ramp.width * 0.5f, ramp.basePos.y + ramp.height, ramp.basePos.z + ramp.length * 0.5f});
//            ramp.worldVerts[3] = toV3f({ramp.basePos.x + ramp.width * 0.5f, ramp.basePos.y + ramp.height, ramp.basePos.z + ramp.length * 0.5f});
//            ramp.worldVerts[4] = toV3f({ramp.basePos.x - ramp.width * 0.5f, ramp.basePos.y, ramp.basePos.z + ramp.length * 0.5f});
//            ramp.worldVerts[5] = toV3f({ramp.basePos.x + ramp.width * 0.5f, ramp.basePos.y, ramp.basePos.z + ramp.length * 0.5f});
//            gjk3df_init_mesh(&ramp.gjkShape, ramp.worldVerts, 6);
//        }
//        mat44_t m = mat44_translate(ramp.basePos.x, ramp.basePos.y, ramp.basePos.z);
//        gModelMatrices.push_back(m);
//        gDrawMeshes.push_back({ gRampVertexBuffer, 24 });
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
//            gDrawMeshes.push_back({ gUnitCubeVertexBuffer, 36 });
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
//            gDrawMeshes.push_back({ gUnitCubeVertexBuffer, 36 });
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
//            gDrawMeshes.push_back({ gUnitCubeVertexBuffer, 36 });
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
//            mat44f_t transform = mat44f_mul(r, t);
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
//            gDrawMeshes.push_back({ gUnitCubeVertexBuffer, 36 });
//        }
//    }
//
//    createFloorTexture();
//    createEnemyBulletTextures();
//    gDepthTexture = render_create_depth_texture(gDevice, WINDOW_WIDTH, WINDOW_HEIGHT);
//    gSampler = render_create_sampler(gDevice);
//    createBulletMesh();
//
//    // 加载 glTF 模型（将 .glb/.gltf 文件放到 joy_editor/ 目录下）
//    //loadGLTFModel("joy_editor_models/air.glb", &gGLTFVertexBuffer, &gGLTFVertexCount);
//    loadGLTFModel("joy_editor_models/robot.glb", &gGLTFVertexBuffer, &gGLTFVertexCount);
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
//    // 在标题栏显示当前动画
//    if (gGLTFValid && gGLTFData) {
//        char title[256];
//        if (gGLTFData->animations_count > 0)
//            snprintf(title, sizeof(title), "Robot (%zu animations) - Press K to cycle", gGLTFData->animations_count);
//        else
//            snprintf(title, sizeof(title), "Robot (no animations)");
//        SDL_SetWindowTitle(gWindow, title);
//    }
//
//    return SDL_APP_CONTINUE;
//}
//
// SDL_AppResult SDL_AppIterate(void* appstate) {
//    auto now = std::chrono::high_resolution_clock::now();
//    gDeltaTime = std::chrono::duration<float>(now - gLastFrameTime).count();
//    gLastFrameTime = now;
//
//    processInput();
//    updateMVPData();
//
//    // ----- glTF 动画更新 -----
//    if (gGLTFValid && gGLTFAnimPlaying && gGLTFData && gGLTFData->animations_count > 0) {
//        gGLTFAnimTime += gDeltaTime;
//
//        // 计算动画持续时间并循环
//        cgltf_animation* anim = &gGLTFData->animations[gGLTFAnimIndex];
//        float maxTime = 0.0f;
//        for (cgltf_size ci = 0; ci < anim->channels_count; ci++) {
//            cgltf_animation_sampler* samp = anim->channels[ci].sampler;
//            if (samp && samp->input && samp->input->buffer_view && samp->input->count > 0) {
//                float* times = (float*)((const uint8_t*)samp->input->buffer_view->buffer->data
//                    + samp->input->offset + samp->input->buffer_view->offset);
//                float last = times[samp->input->count - 1];
//                if (last > maxTime) maxTime = last;
//            }
//        }
//        if (maxTime > 0.0f) gGLTFAnimTime = fmodf(gGLTFAnimTime, maxTime);
//
//        // 1. 重置所有节点到 rest pose
//        for (cgltf_size n = 0; n < gGLTFData->nodes_count; n++) {
//            cgltf_node* node = &gGLTFData->nodes[n];
//            node->translation[0] = 0; node->translation[1] = 0; node->translation[2] = 0;
//            node->rotation[0] = 0; node->rotation[1] = 0; node->rotation[2] = 0; node->rotation[3] = 1;
//            node->scale[0] = 1; node->scale[1] = 1; node->scale[2] = 1;
//            if (node->has_translation) { node->translation[0]=node->translation[1]=node->translation[2]=0; }
//            if (node->has_rotation) { node->rotation[0]=node->rotation[1]=node->rotation[2]=0; node->rotation[3]=1; }
//            if (node->has_scale) { node->scale[0]=node->scale[1]=node->scale[2]=1; }
//        }
//
//        // 2. 应用动画通道
//        float t = gGLTFAnimTime;
//        for (cgltf_size ci = 0; ci < anim->channels_count; ci++) {
//            cgltf_animation_channel* ch = &anim->channels[ci];
//            cgltf_animation_sampler* samp = ch->sampler;
//            if (!samp || !samp->input || !samp->output) continue;
//            cgltf_node* target = ch->target_node;
//            if (!target) continue;
//
//            float* times = (float*)((const uint8_t*)samp->input->buffer_view->buffer->data
//                + samp->input->offset + samp->input->buffer_view->offset);
//            cgltf_size numFrames = samp->input->count;
//
//            // 查找当前帧
//            cgltf_size idx = 0;
//            for (cgltf_size fi = 0; fi < numFrames - 1; fi++) {
//                if (t >= times[fi] && t < times[fi + 1]) { idx = fi; break; }
//                if (fi == numFrames - 2 && t >= times[fi + 1]) idx = fi;
//            }
//            float lerp = (times[idx + 1] - times[idx] > 0.0001f)
//                ? (t - times[idx]) / (times[idx + 1] - times[idx]) : 0.0f;
//
//            if (ch->target_path == cgltf_animation_path_type_translation) {
//                float* va = (float*)((const uint8_t*)samp->output->buffer_view->buffer->data
//                    + samp->output->offset + samp->output->buffer_view->offset);
//                int stride = (samp->output->type == cgltf_type_vec3) ? 3 : 3;
//                target->translation[0] = va[idx * stride + 0] + (va[(idx + 1) * stride + 0] - va[idx * stride + 0]) * lerp;
//                target->translation[1] = va[idx * stride + 1] + (va[(idx + 1) * stride + 1] - va[idx * stride + 1]) * lerp;
//                target->translation[2] = va[idx * stride + 2] + (va[(idx + 1) * stride + 2] - va[idx * stride + 2]) * lerp;
//            } else if (ch->target_path == cgltf_animation_path_type_rotation) {
//                float* qa = (float*)((const uint8_t*)samp->output->buffer_view->buffer->data
//                    + samp->output->offset + samp->output->buffer_view->offset);
//                int stride = (samp->output->type == cgltf_type_vec4) ? 4 : 4;
//                // 简单 lerp 四元数（省略规范化）
//                target->rotation[0] = qa[idx * stride + 0] + (qa[(idx + 1) * stride + 0] - qa[idx * stride + 0]) * lerp;
//                target->rotation[1] = qa[idx * stride + 1] + (qa[(idx + 1) * stride + 1] - qa[idx * stride + 1]) * lerp;
//                target->rotation[2] = qa[idx * stride + 2] + (qa[(idx + 1) * stride + 2] - qa[idx * stride + 2]) * lerp;
//                target->rotation[3] = qa[idx * stride + 3] + (qa[(idx + 1) * stride + 3] - qa[idx * stride + 3]) * lerp;
//            } else if (ch->target_path == cgltf_animation_path_type_scale) {
//                float* va = (float*)((const uint8_t*)samp->output->buffer_view->buffer->data
//                    + samp->output->offset + samp->output->buffer_view->offset);
//                int stride = (samp->output->type == cgltf_type_vec3) ? 3 : 3;
//                target->scale[0] = va[idx * stride + 0] + (va[(idx + 1) * stride + 0] - va[idx * stride + 0]) * lerp;
//                target->scale[1] = va[idx * stride + 1] + (va[(idx + 1) * stride + 1] - va[idx * stride + 1]) * lerp;
//                target->scale[2] = va[idx * stride + 2] + (va[(idx + 1) * stride + 2] - va[idx * stride + 2]) * lerp;
//            }
//        }
//
//        // 3. 更新顶点缓冲区
//        if (!gMeshParts.empty() && gGLTFVertexBuffer) {
//            std::vector<Vertex> animatedVerts = gRestPoseVerts;
//            for (auto& part : gMeshParts) {
//                if (!part.node) continue;
//                // 计算节点世界矩阵（从根到该节点的链）
//                mat44_t worldMatrix = mat44_identity();
//                cgltf_node* n = part.node;
//                while (n) {
//                    mat44_t local = mat44_identity();
//                    float q[4] = { n->rotation[0], n->rotation[1], n->rotation[2], n->rotation[3] };
//                    float len = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
//                    if (len > 0.0001f) { q[0]/=len; q[1]/=len; q[2]/=len; q[3]/=len; }
//                    // 四元数 → 旋转矩阵
//                    float xx = q[0]*q[0], yy = q[1]*q[1], zz = q[2]*q[2];
//                    float xy = q[0]*q[1], xz = q[0]*q[2], xw = q[0]*q[3];
//                    float yz = q[1]*q[2], yw = q[1]*q[3], zw = q[2]*q[3];
//                    local.m0 = (1 - 2*(yy+zz)) * n->scale[0];
//                    local.m1 = (2*(xy+zw)) * n->scale[0];
//                    local.m2 = (2*(xz-yw)) * n->scale[0];
//                    local.m4 = (2*(xy-zw)) * n->scale[1];
//                    local.m5 = (1 - 2*(xx+zz)) * n->scale[1];
//                    local.m6 = (2*(yz+xw)) * n->scale[1];
//                    local.m8 = (2*(xz+yw)) * n->scale[2];
//                    local.m9 = (2*(yz-xw)) * n->scale[2];
//                    local.m10 = (1 - 2*(xx+yy)) * n->scale[2];
//                    local.m12 = n->translation[0];
//                    local.m13 = n->translation[1];
//                    local.m14 = n->translation[2];
//                    local.m15 = 1.0f;
//                    // 累乘 world = local * world（左手系）
//                    // [row * col convention: m[row][col]]
//                    // local * world 表示先应用父级变换再应用本地变换
//                    mat44_t tmp;
//                    for (int r = 0; r < 4; r++)
//                        for (int c = 0; c < 4; c++)
//                            ((float*)&tmp)[r*4+c] = ((float*)&local)[r*4+0]*((float*)&worldMatrix)[0*4+c]
//                                                  + ((float*)&local)[r*4+1]*((float*)&worldMatrix)[1*4+c]
//                                                  + ((float*)&local)[r*4+2]*((float*)&worldMatrix)[2*4+c]
//                                                  + ((float*)&local)[r*4+3]*((float*)&worldMatrix)[3*4+c];
//                    worldMatrix = tmp;
//                    n = n->parent;
//                }
//
//                // 应用世界矩阵到顶点
//                for (int vi = part.vertexOffset; vi < part.vertexOffset + part.vertexCount; vi++) {
//                    Vertex& v = animatedVerts[vi];
//                    Vertex& rest = gRestPoseVerts[vi];
//                    // rest pose 位置是模型空间下的原始位置
//                    // 如果动画改变了节点变换，相对 worldMatrix 就是节点从 rest → animated 的变化
//                    // 实际上 worldMatrix 已经是节点当前的变换，而 rest pose 的顶点是在该节点局部空间下
//                    // 所以直接应用 worldMatrix
//                    float ix = rest.x, iy = rest.y, iz = rest.z;
//                    v.x = worldMatrix.m0 * ix + worldMatrix.m4 * iy + worldMatrix.m8 * iz + worldMatrix.m12;
//                    v.y = worldMatrix.m1 * ix + worldMatrix.m5 * iy + worldMatrix.m9 * iz + worldMatrix.m13;
//                    v.z = worldMatrix.m2 * ix + worldMatrix.m6 * iy + worldMatrix.m10 * iz + worldMatrix.m14;
//                    v.u = rest.u; v.v = rest.v; // 保留 UV
//                }
//            }
//
//            // 上传到 GPU（用临时 transfer buffer）
//            size_t uploadSize = animatedVerts.size() * sizeof(Vertex);
//            SDL_GPUTransferBufferCreateInfo tbci{};
//            tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
//            tbci.size = (Uint32)uploadSize;
//            SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(gDevice, &tbci);
//            memcpy(SDL_MapGPUTransferBuffer(gDevice, tb, false), animatedVerts.data(), uploadSize);
//            SDL_UnmapGPUTransferBuffer(gDevice, tb);
//
//            SDL_GPUCommandBuffer* cb = SDL_AcquireGPUCommandBuffer(gDevice);
//            SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cb);
//            SDL_GPUTransferBufferLocation src{};
//            src.transfer_buffer = tb; src.offset = 0;
//            SDL_GPUBufferRegion dst{};
//            dst.buffer = gGLTFVertexBuffer;
//            dst.offset = 0;
//            dst.size = (Uint32)uploadSize;
//            SDL_UploadToGPUBuffer(cp, &src, &dst, false);
//            SDL_EndGPUCopyPass(cp);
//            SDL_SubmitGPUCommandBuffer(cb);
//            SDL_ReleaseGPUTransferBuffer(gDevice, tb);
//        }
//    }
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
//        render_mat44_to_float16(&gModelMatrices[i], gMVP.model);
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
//    // 绘制 glTF 模型（ANIMATED - 动画每帧修改顶点）
//    if (gGLTFValid) {
//        mat44_t gltfM = mat44_identity();
//        // 模型位于原点，放缩至合适大小
//        gltfM.m0 = 0.5f; gltfM.m5 = 0.5f; gltfM.m10 = 0.5f;
//        gltfM.m12 = 0; gltfM.m13 = 0.0f; gltfM.m14 = -3;
//        render_mat44_to_float16(&gltfM, gMVP.model);
//        SDL_PushGPUVertexUniformData(cmd, 0, &gMVP, sizeof(gMVP));
//
//        SDL_GPUTextureSamplerBinding samp{};
//        samp.texture = gGLTFTexture ? gGLTFTexture : gTextureBullet;
//        samp.sampler = gSampler;
//        SDL_BindGPUFragmentSamplers(rp, 0, &samp, 1);
//
//        SDL_GPUBufferBinding bind{};
//        bind.buffer = gGLTFVertexBuffer;
//        bind.offset = 0;
//        SDL_BindGPUVertexBuffers(rp, 0, &bind, 1);
//        SDL_DrawGPUPrimitives(rp, gGLTFVertexCount, 1, 0, 0);
//    }
//
//    /* 敌人已屏蔽*/
//    for (auto& e : gEnemies) {
//        if (!e.alive) continue;
//        mat44_t m = mat44_identity();
//        m.m0 = 0.5f; m.m5 = 0.5f; m.m10 = 0.5f;
//        m.m12 = e.pos.x; m.m13 = e.pos.y; m.m14 = e.pos.z;
//        render_mat44_to_float16(&m, gMVP.model);
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
//
//    // 绘制子弹（黄色小立方体）
//    for (auto& b : gBullets) {
//        mat44_t m = mat44_identity();
//        m.m0 = 0.08f; m.m5 = 0.08f; m.m10 = 0.08f;
//        m.m12 = b.pos.x; m.m13 = b.pos.y; m.m14 = b.pos.z;
//        render_mat44_to_float16(&m, gMVP.model);
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
//    // 窗口标题显示当前动画
//    if (gGLTFValid && gGLTFData && gGLTFData->animations_count > 0) {
//        char title[256];
//        const char* name = gGLTFData->animations[gGLTFAnimIndex].name;
//        float fps = gGLTFAnimPlaying ? (1.0f / std::max(gDeltaTime, 0.0001f)) : 0.0f;
//        snprintf(title, sizeof(title), "Robot - Anim %d/%zu: %s  |  %d verts %zu parts",
//            gGLTFAnimIndex + 1, gGLTFData->animations_count,
//            name ? name : "(unnamed)",
//            gGLTFVertexCount, gMeshParts.size());
//        SDL_SetWindowTitle(gWindow, title);
//    }
//
//    return SDL_APP_CONTINUE;
// }
//
// SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//
//    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
//        return SDL_APP_SUCCESS;
//    }
//
//    // 键盘 K → 切换下一个 glTF 动画
//    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_K) {
//        if (gGLTFData && gGLTFData->animations_count > 0) {
//            gGLTFAnimIndex = (gGLTFAnimIndex + 1) % (int)gGLTFData->animations_count;
//            gGLTFAnimTime = 0.0f;
//            gGLTFAnimPlaying = true;
//            SDL_Log("Animation %d/%zu: %s", gGLTFAnimIndex + 1, gGLTFData->animations_count,
//                gGLTFData->animations[gGLTFAnimIndex].name ? gGLTFData->animations[gGLTFAnimIndex].name : "(unnamed)");
//        }
//    }
//
//    // 鼠标移动 → 水平旋转玩家朝向（绕 Y 轴）
//    if (event->type == SDL_EVENT_MOUSE_MOTION) {
//        gPlayerYaw += event->motion.xrel * 0.05f;
//    }
//
//    return SDL_APP_CONTINUE;
// }
//
// void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    SDL_WaitForGPUIdle(gDevice);
//    SDL_ReleaseGPUSampler(gDevice, gSampler);
//    SDL_ReleaseGPUTexture(gDevice, gTexture);
//    SDL_ReleaseGPUTexture(gDevice, gTextureEnemy);
//    SDL_ReleaseGPUTexture(gDevice, gTextureBullet);
//    if (gTextureBall) SDL_ReleaseGPUTexture(gDevice, gTextureBall);
//    if (gTextureOBB) SDL_ReleaseGPUTexture(gDevice, gTextureOBB);
//    SDL_ReleaseGPUTexture(gDevice, gDepthTexture);
//    SDL_ReleaseGPUBuffer(gDevice, gFloorVertexBuffer);
//    SDL_ReleaseGPUBuffer(gDevice, gCubeVertexBuffer);
//    if (gUnitCubeVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gUnitCubeVertexBuffer);
//    if (gRampVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gRampVertexBuffer);
//    if (gBulletVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gBulletVertexBuffer);
//    if (gSphereVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gSphereVertexBuffer);
//    if (gGLTFVertexBuffer) SDL_ReleaseGPUBuffer(gDevice, gGLTFVertexBuffer);
//    if (gGLTFTexture) SDL_ReleaseGPUTexture(gDevice, gGLTFTexture);
//    if (gGLTFData) { cgltf_free(gGLTFData); gGLTFData = nullptr; }
//    SDL_ReleaseGPUGraphicsPipeline(gDevice, gGraphicsPipeline);
//    SDL_ReleaseGPUShader(gDevice, gShaders.vertex);
//    SDL_ReleaseGPUShader(gDevice, gShaders.fragment);
//    SDL_ReleaseWindowFromGPUDevice(gDevice, gWindow);
//    SDL_DestroyGPUDevice(gDevice);
//    SDL_DestroyWindow(gWindow);
// }
