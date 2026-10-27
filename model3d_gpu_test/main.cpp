/*
 * model3d_gpu_test —— SDL3 GPU API 加载 / 渲染 3D 模型测试（独立可执行程序）
 *
 * - 渲染：直接使用 SDL3 GPU API（SDL_gpu.h，SDL_GPUDevice 全家桶），不经过 SDL_Renderer。
 *   着色器复用 SDL 官方 testgpu 的预编译产物（cube.vert/frag 的 DXIL/SPIR-V/MSL 三格式，
 *   运行时按 SDL_GetGPUShaderFormats 选择），因此本机无需安装 dxc / glslc / shadercross 工具链。
 *   着色器约定：顶点属性 position(FLOAT3, loc=0) + color(FLOAT3, loc=1)；
 *   顶点 uniform 为一个 float4x4 MVP（列主序），通过 SDL_PushGPUVertexUniformData 上传。
 * - 模型：内置简易 OBJ 解析器（支持 v/vn/f、负索引、多顶点面 fan 三角化），
 *   glTF 2.0（.glb/.gltf）由 assimp 解析（节点层级、蒙皮骨骼、动画、纯色材质色）；
 *   默认加载 assets/models/robot.glb（也可带 robot 用 -m 指定 .obj），
 *   加载失败时程序化生成 UV 球体兜底，保证开箱即用。
 * - 颜色：CPU 端由法线生成伪彩色 + 假光照（Lambert），无需纹理。
 *
 * 用法：
 *   model3d_gpu_test [model.obj|model.glb]       # 缺省加载 models/robot.glb
 *   model3d_gpu_test --driver vulkan [model.obj] # 强制指定后端（d3d12/vulkan/metal）
 *   后端也可用 SDL_GPU_DRIVER 环境变量覆盖；默认优先 vulkan，回退 d3d12。
 *
 * 操作：
 *   空格        切换自动旋转
 *   方向键      手动旋转（上下=俯仰，左右=偏航）
 *   A / D       绕 Z 轴旋转（滚转）
 *   W / S       拉近 / 拉远
 *   F           线框 / 填充 切换
 *   R           重置视角
 *   Esc / 关窗  退出
 */
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/* 复用 SDL 官方 testgpu 预编译着色器头（路径由 CMake 的 SDL/test 包含目录提供） */
#include "testgpu/cube.vert.dxil.h"
#include "testgpu/cube.vert.spv.h"
#include "testgpu/cube.vert.msl.h"
#include "testgpu/cube.frag.dxil.h"
#include "testgpu/cube.frag.spv.h"
#include "testgpu/cube.frag.msl.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#define WINDOW_WIDTH  960
#define WINDOW_HEIGHT 640

#define SUPPORTED_SHADER_FORMATS \
        (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | \
         SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)

/* ==================== 极小 3D 数学（列主序 float[16]） ==================== */

static void rotate_matrix(float angle, float x, float y, float z, float *r)
{
        float radians = angle * SDL_PI_F / 180.0f;
        float c = SDL_cosf(radians), s = SDL_sinf(radians);
        float c1 = 1.0f - c;
        float length = SDL_sqrtf(x * x + y * y + z * z);
        float u[3] = {x / length, y / length, z / length};
        int i, j;

        for (i = 0; i < 16; i++) r[i] = 0.0f;
        r[15] = 1.0f;

        /* 标准罗德里格斯旋转（列主序） */
        for (i = 0; i < 3; i++) {
                r[i * 4 + (i + 1) % 3] =  u[(i + 2) % 3] * s;
                r[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
        }
        for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) {
                        r[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0f);
                }
        }
}

static void scale_matrix(float sx, float sy, float sz, float *r)
{
        for (int i = 0; i < 16; i++) r[i] = 0.0f;
        r[0] = sx; r[5] = sy; r[10] = sz; r[15] = 1.0f;
}

static void perspective_matrix(float fovy, float aspect, float znear, float zfar, float *r)
{
        float f = 1.0f / SDL_tanf((fovy / 180.0f) * SDL_PI_F * 0.5f);
        int i;
        for (i = 0; i < 16; i++) r[i] = 0.0f;
        r[0] = f / aspect;
        r[5] = f;
        r[10] = (znear + zfar) / (znear - zfar);
        r[11] = -1.0f;
        r[14] = (2.0f * znear * zfar) / (znear - zfar);
        r[15] = 0.0f;
}

/* 列主序 lhs * rhs -> r（支持原地） */
static void multiply_matrix(const float *lhs, const float *rhs, float *r)
{
        int i, j, k;
        float tmp[16];
        for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                        tmp[j * 4 + i] = 0.0f;
                        for (k = 0; k < 4; k++) {
                                tmp[j * 4 + i] += lhs[k * 4 + i] * rhs[j * 4 + k];
                        }
                }
        }
        for (i = 0; i < 16; i++) r[i] = tmp[i];
}

/* ==================== OBJ 解析 + 模型数据 ==================== */

/* ---- glTF 骨骼 / 动画运行时数据 ---- */

/* 节点本地变换：优先 matrix（has_matrix=true），否则 TRS（t3+r4+s3） */
struct GltfNode {
        float trs[10] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1};  /* t0..2, r0..3(w), s0..2 */
        float matrix[16];
        bool has_matrix = false;
        std::vector<int> children;
};

struct GltfSkin {
        std::vector<int> joints;        /* 关节 = nodes 下标 */
        std::vector<float> inv_bind;    /* 每关节 16 float（列主序逆绑定矩阵） */
};

struct GltfAnim {
        struct Channel { int sampler = -1; int node = -1; int path = 0; };  /* path: 0=T 1=R 2=S 3=WEIGHTS */
        struct Sampler {
                std::vector<float> times;
                std::vector<float> values;
                int comps = 3;          /* 每关键帧分量数 */
                int interp = 0;         /* 0=LINEAR 1=STEP 2=CUBICSPLINE */
        };
        std::vector<Channel> channels;
        std::vector<Sampler> samplers;
        float duration = 1.0f;
};

struct Model {
        std::vector<float> raw_pos;   /* 每三角形 9 个 float（3 顶点 xyz；每帧被蒙皮结果覆盖） */
        std::vector<float> raw_nrm;   /* 每三角形 9 个 float（预设法线，可为空） */
        std::vector<float> bind_pos;  /* 绑定姿势顶点备份（加载后一次性保存，蒙皮永远从这里读输入） */
        std::vector<float> bind_nrm;  /* 绑定姿势法线备份（同上） */
        std::vector<uint16_t> joints; /* 每三角形顶点 4 个关节索引（仅 skinned；与 raw_pos 顶点一一对应） */
        std::vector<float> weights;   /* 每三角形顶点 4 个权重 */
        std::vector<float> colors;    /* 每三角形顶点 rgb 材质色（可为空 -> 用伪彩色） */
        std::vector<float> verts;     /* 交错 pos(3)+color(3)，上传 GPU 用 */
        Uint32 index_count = 0;       /* 顶点数 = 三角形数 * 3 */

        bool skinned = false;
        std::vector<GltfNode> nodes;
        std::vector<GltfSkin> skins;
        std::vector<GltfAnim> anims;
        std::vector<std::string> anim_names;   /* 与 anims 一一对应的动画名 */
        std::vector<int> scene_roots;   /* 场景根节点下标（计算全局矩阵的起点） */
        std::vector<float> node_global;  /* 每节点 16 float 全局矩阵（每帧重算） */
        std::vector<float> joint_mat;    /* 每关节 16 float = nodeGlobal[joint] * invBind[joint] */
        float anim_time = 0.0f;
        float anim_speed = 1.0f;
        int anim_index = 0;         /* 当前播放动画下标（anims 内，超出自动取模） */
};

struct Vec3 { float x, y, z; };

static Vec3 v3(float x, float y, float z) { Vec3 v = {x, y, z}; return v; }
static Vec3 v3sub(const Vec3 &a, const Vec3 &b) { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
static Vec3 v3add(const Vec3 &a, const Vec3 &b) { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
static Vec3 v3scale(const Vec3 &a, float s) { return v3(a.x * s, a.y * s, a.z * s); }
static float v3dot(const Vec3 &a, const Vec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 v3cross(const Vec3 &a, const Vec3 &b) {
        return v3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
static Vec3 v3normalize(const Vec3 &a) {
        float len = SDL_sqrtf(v3dot(a, a));
        return len > 1e-8f ? v3scale(a, 1.0f / len) : v3(0, 0, 1);
}

/* 从顶点索引（1 起始 / 负索引）解析出 OBJ 内部下标（0 起始），失败返回 -1 */
static int resolve_index(int idx, int count)
{
        if (count <= 0) return -1;
        if (idx > 0) return idx - 1;
        if (idx < 0) return count + idx;   /* -1 => 最后一个 */
        return -1;
}

static bool load_obj(const char *path, Model &m, std::string &err)
{
        std::vector<Vec3> vlist, vnlist;
        std::vector<int> faces;   /* 每面依次 3 个顶点下标（fan 三角化后的三角形） */
        FILE *fp = fopen(path, "rb");
        if (!fp) { err = "cannot open file"; return false; }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
                char tag[16];
                if (sscanf(line, "%15s", tag) != 1) continue;
                if (strcmp(tag, "v") == 0) {
                        float x, y, z;
                        if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) vlist.push_back(v3(x, y, z));
                } else if (strcmp(tag, "vn") == 0) {
                        float x, y, z;
                        if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) vnlist.push_back(v3(x, y, z));
                } else if (strcmp(tag, "f") == 0) {
                        /* 解析 "f v[/vt][/vn] ..."，支持 3~N 个顶点，fan 三角化 */
                        std::vector<int> face;
                        char *p = line + 1;   /* 跳过 'f' */
                        while (*p) {
                                int vi = 0, ti = 0, ni = 0;
                                int n = sscanf(p, " %d/%d/%d", &vi, &ti, &ni);
                                if (n < 1 || vi == 0) { /* 未匹配到完整 v/vt/vn 时回退重试 */
                                        n = sscanf(p, " %d/%d", &vi, &ti);
                                        if (n < 1) { n = sscanf(p, " %d", &vi); }
                                        ni = 0;
                                }
                                if (n < 1 || vi == 0) break;
                                face.push_back(vi);
                                while (*p && *p != ' ') p++;
                                while (*p == ' ') p++;
                        }
                        if (face.size() >= 3) {
                                for (size_t k = 1; k + 1 < face.size(); k++) {
                                        faces.push_back(face[0]);
                                        faces.push_back(face[k]);
                                        faces.push_back(face[k + 1]);
                                }
                        }
                }
        }
        fclose(fp);

        if (vlist.empty() || faces.empty()) { err = "no vertices/faces in obj"; return false; }

        /* 法线优先用 vn（若该三角形三个角均能解析到 vn 且一致）；否则叉积 + 质心修正 */
        bool has_vn = vnlist.size() == vlist.size(); /* 仅当每个顶点都有法线时才采用 vn */
        Vec3 center = v3(0, 0, 0);
        for (const Vec3 &v : vlist) center = v3add(center, v);
        center = v3scale(center, 1.0f / (float)vlist.size());

        for (size_t i = 0; i + 2 < faces.size(); i += 3) {
                Vec3 p[3];
                bool ok = true;
                for (int k = 0; k < 3; k++) {
                        int idx = resolve_index(faces[i + k], (int)vlist.size());
                        if (idx < 0) { ok = false; break; }
                        p[k] = vlist[idx];
                }
                if (!ok) { err = "bad face index in obj"; return false; }

                Vec3 n;
                if (has_vn) {
                        int idx = resolve_index(faces[i], (int)vnlist.size());
                        n = v3normalize(vnlist[idx]);
                } else {
                        n = v3normalize(v3cross(v3sub(p[1], p[0]), v3sub(p[2], p[0])));
                        /* 质心朝向修正：法线应指向模型包围中心外侧 */
                        Vec3 centroid = v3scale(v3add(v3add(p[0], p[1]), p[2]), 1.0f / 3.0f);
                        if (v3dot(n, v3sub(centroid, center)) < 0.0f) n = v3scale(n, -1.0f);
                }
                for (int k = 0; k < 3; k++) {
                        m.raw_pos.push_back(p[k].x); m.raw_pos.push_back(p[k].y); m.raw_pos.push_back(p[k].z);
                        m.raw_nrm.push_back(n.x);    m.raw_nrm.push_back(n.y);    m.raw_nrm.push_back(n.z);
                }
        }
        return true;
}

/* 程序化 UV 球体（无纹理坐标），法线直接取顶点单位向量 */
/* ==================== glTF 2.0 (.glb/.gltf) 解析器（assimp） ==================== */

static void identity_matrix(float m[16])
{
        for (int i = 0; i < 16; i++) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
}

/* 把 aiMatrix4x4（行主序）转置为列主序 float[16] */
static void assimp_mat_to_col(const aiMatrix4x4 &src, float dst[16])
{
        dst[0] = src.a1; dst[4] = src.a2; dst[8] = src.a3;  dst[12] = src.a4;
        dst[1] = src.b1; dst[5] = src.b2; dst[9] = src.b3;  dst[13] = src.b4;
        dst[2] = src.c1; dst[6] = src.c2; dst[10] = src.c3; dst[14] = src.c4;
        dst[3] = src.d1; dst[7] = src.d2; dst[11] = src.d3; dst[15] = src.d4;
}



/* 列主序 4x4 矩阵变换（位置带平移；法线仅用旋转部分） */
static Vec3 transform_point(const float m[16], const Vec3 &v)
{
        return v3(m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12],
                  m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],
                  m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]);
}
static Vec3 transform_normal(const float m[16], const Vec3 &n)
{
        return v3(m[0] * n.x + m[4] * n.y + m[8] * n.z,
                  m[1] * n.x + m[5] * n.y + m[9] * n.z,
                  m[2] * n.x + m[6] * n.y + m[10] * n.z);
}

static void quat_to_matrix(float x, float y, float z, float w, float m[16])
{
        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;
        /* 列主序：m[c*4+r] 第 c 列第 r 行，与 rotate_matrix/transform_point/multiply_matrix 约定一致。
         * 之前误用行主序布局，导致旋转矩阵被转置（=逆旋转），关节集体反向，姿态错乱。 */
        m[0]  = 1 - 2 * (yy + zz);  m[4]  = 2 * (xy - wz);    m[8]  = 2 * (xz + wy);  m[12] = 0;
        m[1]  = 2 * (xy + wz);      m[5]  = 1 - 2 * (xx + zz); m[9]  = 2 * (yz - wx);  m[13] = 0;
        m[2]  = 2 * (xz - wy);      m[6]  = 2 * (yz + wx);    m[10] = 1 - 2 * (xx + yy); m[14] = 0;
        m[3]  = 0;                  m[7]  = 0;                m[11] = 0;               m[15] = 1;
}

/* 运行时 GltfNode 局部矩阵（has_matrix 优先，否则 T*R*S） */
static void node_matrix_from_trs(const GltfNode &gn, float m[16])
{
        if (gn.has_matrix) { for (int i = 0; i < 16; i++) m[i] = gn.matrix[i]; return; }
        float R[16], T[16], S[16], tmp[16];
        quat_to_matrix(gn.trs[3], gn.trs[4], gn.trs[5], gn.trs[6], R);
        identity_matrix(T);
        T[12] = gn.trs[0]; T[13] = gn.trs[1]; T[14] = gn.trs[2];
        identity_matrix(S);
        S[0] = gn.trs[7]; S[5] = gn.trs[8]; S[10] = gn.trs[9];
        multiply_matrix(R, S, tmp);
        multiply_matrix(T, tmp, m);
}



/* ==================== assimp 数据 -> Model 转换 ==================== */

/* 递归收集 aiNode 树到 Model.nodes（DFS：父先于子，mRootNode=0）。
 * 所有节点存 TRS（has_matrix=false，Decompose 得到），动画 channel 可直接改写 trs。 */
static void collect_nodes_rec(const aiScene *scene, const aiNode *an, Model &m,
                              std::vector<const aiNode *> &ptrs, int parent_idx)
{
        int idx = (int)m.nodes.size();
        m.nodes.emplace_back();
        ptrs.push_back(an);
        GltfNode &gn = m.nodes[idx];
        if (parent_idx >= 0) m.nodes[parent_idx].children.push_back(idx);

        aiVector3D sc, tr;
        aiQuaternion rot;
        an->mTransformation.Decompose(sc, rot, tr);
        gn.trs[0] = tr.x; gn.trs[1] = tr.y; gn.trs[2] = tr.z;
        gn.trs[3] = rot.x; gn.trs[4] = rot.y; gn.trs[5] = rot.z; gn.trs[6] = rot.w;
        gn.trs[7] = sc.x; gn.trs[8] = sc.y; gn.trs[9] = sc.z;
        gn.has_matrix = false;

        for (unsigned c = 0; c < an->mNumChildren; c++)
                collect_nodes_rec(scene, an->mChildren[c], m, ptrs, idx);
        (void)scene;
}

/* 建立 aiNode 名字 -> 节点下标 映射（覆盖写避免重名风险：后建的优先） */
static void collect_node_map(const aiScene *scene, const aiNode *an,
                             std::map<std::string, int> &name2idx,
                             const std::vector<const aiNode *> &ptrs)
{
        if (an->mName.length > 0) {
                auto it = name2idx.find(an->mName.C_Str());
                if (it == name2idx.end()) {
                        for (size_t i = 0; i < ptrs.size(); i++)
                                if (ptrs[i] == an) { name2idx[an->mName.C_Str()] = (int)i; break; }
                }
        }
        for (unsigned c = 0; c < an->mNumChildren; c++)
                collect_node_map(scene, an->mChildren[c], name2idx, ptrs);
        (void)scene;
}

/* 收集所有 mesh 的骨骼：按名字去重，得全局唯一骨骼序列（即 GltfSkin.joints） */
static void collect_skin(const aiScene *scene, Model &m,
                         std::map<std::string, int> &bone2node,
                         std::map<std::string, int> &bone2joint)
{
        GltfSkin &gs = m.skins.emplace_back();
        for (unsigned mi = 0; mi < scene->mNumMeshes; mi++) {
                const aiMesh *mesh = scene->mMeshes[mi];
                for (unsigned b = 0; b < mesh->mNumBones; b++) {
                        const aiBone *bone = mesh->mBones[b];
                        const char *bn = bone->mName.C_Str();
                        if (bone2joint.find(bn) != bone2joint.end()) continue;   /* 已收录 */
                        int node = -1;
                        auto it = bone2node.find(bn);
                        if (it != bone2node.end()) node = it->second;
                        gs.joints.push_back(node);
                        float col[16];
                        assimp_mat_to_col(bone->mOffsetMatrix, col);
                        for (int i = 0; i < 16; i++) gs.inv_bind.push_back(col[i]);
                        bone2joint[bn] = (int)gs.joints.size() - 1;
                        m.skinned = true;
                }
        }
}

/* 取材质漫反射/baseColor 色（glTF baseColorFactor 落于 DIFFUSE） */
static void material_color(const aiScene *scene, unsigned mat_idx, float out[3])
{
        out[0] = out[1] = out[2] = 1.0f;
        if (mat_idx >= scene->mNumMaterials) return;
        const aiMaterial *mat = scene->mMaterials[mat_idx];
        aiColor4D c;
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS && (c.r + c.g + c.b) > 1e-6f) {
                out[0] = c.r; out[1] = c.g; out[2] = c.b;
        } else if (mat->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS) {
                out[0] = c.r; out[1] = c.g; out[2] = c.b;
        }
}

/* 递归遍历 aiNode：累计全局矩阵，把每个 mesh 的三角形展开到 Model。
 * skinned mesh：顶点保持绑定姿势本地坐标 + joints/weights（每帧 CPU 蒙皮）；
 * 非 skinned mesh：顶点/法线乘节点全局矩阵。 */
static void process_node_assimp(const aiScene *scene, int node_idx, const float parent[16],
                                Model &m, const std::vector<const aiNode *> &ptrs,
                                const std::map<std::string, int> &bone2joint)
{
        if (node_idx < 0 || node_idx >= (int)m.nodes.size()) return;
        float local[16], combined[16];
        node_matrix_from_trs(m.nodes[node_idx], local);
        multiply_matrix(parent, local, combined);

        const aiNode *an = ptrs[node_idx];
        for (unsigned i = 0; i < an->mNumMeshes; i++) {
                unsigned mi = an->mMeshes[i];
                if (mi >= scene->mNumMeshes) continue;
                const aiMesh *mesh = scene->mMeshes[mi];
                if (!mesh->HasPositions()) continue;
                bool skinned = mesh->mNumBones > 0;
                float mat_color[3];
                material_color(scene, mesh->mMaterialIndex, mat_color);

                /* 顶点蒙皮影响：累加后取权重最大 4 个（assimp 一般每顶点 ≤4） */
                std::vector<float> pj, pw;
                if (skinned) {
                        size_t vc = mesh->mNumVertices;
                        std::vector<std::vector<std::pair<int, float>>> acc(vc);
                        for (unsigned b = 0; b < mesh->mNumBones; b++) {
                                const aiBone *bone = mesh->mBones[b];
                                auto jit = bone2joint.find(bone->mName.C_Str());
                                if (jit == bone2joint.end()) continue;
                                int j = jit->second;
                                for (unsigned w = 0; w < bone->mNumWeights; w++) {
                                        const aiVertexWeight &vw = bone->mWeights[w];
                                        if (vw.mVertexId < vc) acc[vw.mVertexId].push_back({j, vw.mWeight});
                                }
                        }
                        pj.resize(vc * 4, 0); pw.resize(vc * 4, 0.0f);
                        for (size_t v = 0; v < vc; v++) {
                                auto &list = acc[v];
                                std::sort(list.begin(), list.end(),
                                          [](const std::pair<int, float> &a, const std::pair<int, float> &b) {
                                                  return a.second > b.second;
                                          });
                                size_t k = list.size() < 4 ? list.size() : 4;
                                float sum = 0.0f;
                                for (size_t q = 0; q < k; q++) { pj[v * 4 + q] = (float)list[q].first; pw[v * 4 + q] = list[q].second; sum += list[q].second; }
                                if (sum > 0.0f) for (size_t q = 0; q < 4; q++) pw[v * 4 + q] /= sum;
                        }
                }

                for (unsigned f = 0; f < mesh->mNumFaces; f++) {
                        const aiFace &face = mesh->mFaces[f];
                        if (face.mNumIndices != 3) continue;
                        for (int k = 0; k < 3; k++) {
                                unsigned vi = face.mIndices[k];
                                if (vi >= mesh->mNumVertices) continue;
                                aiVector3D p = mesh->mVertices[vi];
                                aiVector3D n = mesh->mNormals[vi];
                                Vec3 bp = v3(p.x, p.y, p.z);
                                Vec3 bn = v3(n.x, n.y, n.z);
                                Vec3 op = skinned ? bp : transform_point(combined, bp);
                                Vec3 on = skinned ? bn : transform_normal(combined, bn);
                                m.raw_pos.push_back(op.x); m.raw_pos.push_back(op.y); m.raw_pos.push_back(op.z);
                                m.raw_nrm.push_back(on.x); m.raw_nrm.push_back(on.y); m.raw_nrm.push_back(on.z);
                                m.colors.push_back(mat_color[0]); m.colors.push_back(mat_color[1]); m.colors.push_back(mat_color[2]);
                                if (skinned) {
                                        for (int q = 0; q < 4; q++) {
                                                m.joints.push_back((uint16_t)pj[vi * 4 + q]);
                                                m.weights.push_back(pw[vi * 4 + q]);
                                        }
                                }
                        }
                }
        }
        for (int child : m.nodes[node_idx].children)
                process_node_assimp(scene, child, combined, m, ptrs, bone2joint);
}

/* glTF(.glb/.gltf) -> Model（assimp；仅解析层，运行时动画/蒙皮沿用原有逻辑） */
static bool load_assimp(const char *path, Model &m, std::string &err)
{
        Assimp::Importer importer;
        /* Triangulate: 面三角化；GenNormals: 无法线时按面生成（skinned 顶点法线需保留原值，
         * GenNormals 只补充缺失项）。JoinIdenticalVertices 会影响蒙皮索引，不启用。 */
        const aiScene *scene = importer.ReadFile(path,
                aiProcess_Triangulate | aiProcess_GenNormals);
        if (!scene || !scene->mRootNode) {
                err = importer.GetErrorString();
                return false;
        }

        /* 1) 节点树 + 名字映射 */
        std::vector<const aiNode *> ptrs;
        collect_nodes_rec(scene, scene->mRootNode, m, ptrs, -1);
        std::map<std::string, int> name2idx;
        collect_node_map(scene, scene->mRootNode, name2idx, ptrs);
        std::map<std::string, int> bone2node;
        for (auto &kv : name2idx) bone2node[kv.first] = kv.second;

        /* 2) 皮肤（按骨骼名去重） */
        std::map<std::string, int> bone2joint;
        collect_skin(scene, m, bone2node, bone2joint);
        if (m.skinned && m.skins[0].joints.empty()) { err = "skinned model with no joints"; return false; }

        /* 3) 网格三角形展开 */
        float ident[16];
        identity_matrix(ident);
        m.scene_roots.push_back(0);   /* assimp mRootNode 即场景根 */
        process_node_assimp(scene, 0, ident, m, ptrs, bone2joint);
        if (m.raw_pos.empty()) { err = "model produced no triangles"; return false; }

        /* 4) 动画：每个 aiNodeAnim 按 path 拆成 1~3 个 channel（T/R/S 各一 sampler） */
        double tps = 1.0;
        for (unsigned a = 0; a < scene->mNumAnimations; a++) {
                const aiAnimation *anim = scene->mAnimations[a];
                GltfAnim &ga = m.anims.emplace_back();
                m.anim_names.emplace_back(anim->mName.length ? anim->mName.C_Str() : "");
                tps = anim->mTicksPerSecond > 0.0 ? anim->mTicksPerSecond : 25.0;
                ga.duration = (float)(anim->mDuration / tps);
                for (unsigned c = 0; c < anim->mNumChannels; c++) {
                        const aiNodeAnim *na = anim->mChannels[c];
                        auto it = name2idx.find(na->mNodeName.C_Str());
                        if (it == name2idx.end()) continue;
                        /* translation */
                        if (na->mNumPositionKeys > 0) {
                                GltfAnim::Sampler s;
                                s.comps = 3;
                                for (unsigned k = 0; k < na->mNumPositionKeys; k++) {
                                        s.times.push_back((float)(na->mPositionKeys[k].mTime / tps));
                                        s.values.push_back(na->mPositionKeys[k].mValue.x);
                                        s.values.push_back(na->mPositionKeys[k].mValue.y);
                                        s.values.push_back(na->mPositionKeys[k].mValue.z);
                                }
                                int si = (int)ga.samplers.size();
                                ga.samplers.push_back(std::move(s));
                                GltfAnim::Channel gc; gc.sampler = si; gc.node = it->second; gc.path = 0;
                                ga.channels.push_back(gc);
                        }
                        /* rotation */
                        if (na->mNumRotationKeys > 0) {
                                GltfAnim::Sampler s;
                                s.comps = 4;
                                for (unsigned k = 0; k < na->mNumRotationKeys; k++) {
                                        s.times.push_back((float)(na->mRotationKeys[k].mTime / tps));
                                        s.values.push_back(na->mRotationKeys[k].mValue.x);
                                        s.values.push_back(na->mRotationKeys[k].mValue.y);
                                        s.values.push_back(na->mRotationKeys[k].mValue.z);
                                        s.values.push_back(na->mRotationKeys[k].mValue.w);
                                }
                                int si = (int)ga.samplers.size();
                                ga.samplers.push_back(std::move(s));
                                GltfAnim::Channel gc; gc.sampler = si; gc.node = it->second; gc.path = 1;
                                ga.channels.push_back(gc);
                        }
                        /* scaling */
                        if (na->mNumScalingKeys > 0) {
                                GltfAnim::Sampler s;
                                s.comps = 3;
                                for (unsigned k = 0; k < na->mNumScalingKeys; k++) {
                                        s.times.push_back((float)(na->mScalingKeys[k].mTime / tps));
                                        s.values.push_back(na->mScalingKeys[k].mValue.x);
                                        s.values.push_back(na->mScalingKeys[k].mValue.y);
                                        s.values.push_back(na->mScalingKeys[k].mValue.z);
                                }
                                int si = (int)ga.samplers.size();
                                ga.samplers.push_back(std::move(s));
                                GltfAnim::Channel gc; gc.sampler = si; gc.node = it->second; gc.path = 2;
                                ga.channels.push_back(gc);
                        }
                }
                printf("[anim %u] name='%s' duration=%.2fs channels=%u\n",
                       a, anim->mName.C_Str(), ga.duration, ga.channels.size());
        }
        /* 关节映射诊断：多少关节找不到对应节点 */
        if (m.skinned) {
                int bad = 0;
                for (int j : m.skins[0].joints) if (j < 0) bad++;
                printf("[skin] joints=%zu (invalid nodes=%d)\n", m.skins[0].joints.size(), bad);
        }
        return true;
}

/* 按扩展名分发：.glb/.gltf -> assimp，其余 -> OBJ 解析器 */
static bool load_model(const char *path, Model &m, std::string &err)
{
        const char *dot = strrchr(path, '.');
        if (dot && (SDL_strcasecmp(dot, ".glb") == 0 || SDL_strcasecmp(dot, ".gltf") == 0)) {
                return load_assimp(path, m, err);
        }
        return load_obj(path, m, err);
}

/* ==================== 动画驱动 + CPU 蒙皮（运行时每帧） ==================== */

/* 采样关键帧：LINEAR 插值 / STEP 取前帧；t 超出范围时夹到首尾 */
static void sample_sampler(const GltfAnim::Sampler &s, float t, float *out)
{
        size_t k = s.times.size();
        if (k == 0) return;
        if (t <= s.times[0] || k == 1) {
                for (int c = 0; c < s.comps; c++) out[c] = s.values[c];
                return;
        }
        if (t >= s.times[k - 1]) {
                for (int c = 0; c < s.comps; c++) out[c] = s.values[(k - 1) * s.comps + c];
                return;
        }
        size_t i = 1;
        while (i + 1 < k && s.times[i] < t) i++;
        float t0 = s.times[i - 1], t1 = s.times[i];
        float f = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;
        if (s.interp == 1) f = 0.0f;   /* STEP */
        const float *a = &s.values[(i - 1) * s.comps];
        const float *b = &s.values[i * s.comps];
        if (s.comps == 4) {
                /* 四元数（rotation）：最短弧 slerp。直接 nlerp 在符号翻转或大夹角时
                 * 会走长弧，导致骨骼旋转反向/抽搐/爆开（动画"乱"的典型来源）。 */
                float q0[4] = {a[0], a[1], a[2], a[3]};
                float q1[4] = {b[0], b[1], b[2], b[3]};
                float l0 = SDL_sqrtf(q0[0]*q0[0] + q0[1]*q0[1] + q0[2]*q0[2] + q0[3]*q0[3]);
                float l1 = SDL_sqrtf(q1[0]*q1[0] + q1[1]*q1[1] + q1[2]*q1[2] + q1[3]*q1[3]);
                if (l0 > 1e-8f) for (int c = 0; c < 4; c++) q0[c] /= l0;
                if (l1 > 1e-8f) for (int c = 0; c < 4; c++) q1[c] /= l1;
                float dot = q0[0]*q1[0] + q0[1]*q1[1] + q0[2]*q1[2] + q0[3]*q1[3];
                if (dot < 0.0f) { dot = -dot; for (int c = 0; c < 4; c++) q1[c] = -q1[c]; }
                const float eps = 0.9995f;
                if (dot > eps) {
                        /* 几乎同向：nlerp 即可 */
                        for (int c = 0; c < 4; c++) out[c] = q0[c] + (q1[c] - q0[c]) * f;
                        float len = SDL_sqrtf(out[0]*out[0] + out[1]*out[1] + out[2]*out[2] + out[3]*out[3]);
                        if (len > 1e-8f) for (int c = 0; c < 4; c++) out[c] /= len;
                } else {
                        float th = SDL_acosf(dot);
                        float sth = SDL_sinf(th);
                        float w0 = SDL_sinf((1.0f - f) * th) / sth;
                        float w1 = SDL_sinf(f * th) / sth;
                        for (int c = 0; c < 4; c++) out[c] = q0[c] * w0 + q1[c] * w1;
                }
                return;
        }
        for (int c = 0; c < s.comps; c++)
                out[c] = a[c] + (b[c] - a[c]) * f;
}

/* 推进动画时间并写入各 channel 目标节点的 TRS */
static void update_animation(Model &m, float dt)
{
        if (m.anims.empty()) return;
        GltfAnim &ga = m.anims[m.anim_index % (int)m.anims.size()];
        m.anim_time += dt * m.anim_speed;
        if (ga.duration > 0.0f) {
                while (m.anim_time >= ga.duration) m.anim_time -= ga.duration;
                while (m.anim_time < 0.0f) m.anim_time += ga.duration;
        }
        float t = m.anim_time;
        for (const GltfAnim::Channel &gc : ga.channels) {
                if (gc.sampler < 0 || gc.sampler >= (int)ga.samplers.size()) continue;
                if (gc.node < 0 || gc.node >= (int)m.nodes.size()) continue;
                const GltfAnim::Sampler &s = ga.samplers[gc.sampler];
                if (s.times.empty() || s.values.empty() || s.comps < 1) continue;
                std::vector<float> v(s.comps);
                sample_sampler(s, t, v.data());
                GltfNode &gn = m.nodes[gc.node];
                if (gc.path == 0 && s.comps >= 3) {
                        gn.trs[0] = v[0]; gn.trs[1] = v[1]; gn.trs[2] = v[2];
                } else if (gc.path == 1 && s.comps >= 4) {
                        /* 四元数：线性插值后归一化（nlerp，可接受近似） */
                        float len = SDL_sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
                        if (len > 1e-8f) { v[0] /= len; v[1] /= len; v[2] /= len; v[3] /= len; }
                        for (int q = 0; q < 4; q++) gn.trs[3 + q] = v[q];
                } else if (gc.path == 2 && s.comps >= 3) {
                        gn.trs[7] = v[0]; gn.trs[8] = v[1]; gn.trs[9] = v[2];
                }
                /* path==3（weights / morph target）：本实现跳过 */
        }
}

static void globals_rec(Model &m, int i, const float parent[16])
{
        float cur[16];
        multiply_matrix(parent, &m.node_global[i * 16], cur);
        for (int c = 0; c < 16; c++) m.node_global[i * 16 + c] = cur[c];
        for (int child : m.nodes[i].children) globals_rec(m, child, cur);
}

/* 重算全部节点全局矩阵（场景根递归；关节通常都在场景图中） */
static void recompute_globals(Model &m)
{
        size_t n = m.nodes.size();
        m.node_global.assign(n * 16, 0.0f);
        for (size_t i = 0; i < n; i++) node_matrix_from_trs(m.nodes[i], &m.node_global[i * 16]);
        float ident[16];
        identity_matrix(ident);
        for (int r : m.scene_roots) globals_rec(m, r, ident);
}

/* CPU 蒙皮：顶点 = Σ w_k * jointMatrix[joint_k] * bindPos；法线同样（仅旋转缩放部分）。
 * 输入恒为绑定位姿 bind_pos/bind_nrm（审计见：此前直接从 raw_pos 读又在原地覆盖，
 * 导致每帧在上一帧蒙皮结果上叠加，越播越乱）。 */
static void skin_model(Model &m)
{
        size_t nv = m.raw_pos.size() / 3;
        if (!m.skinned || m.skins.empty()) return;
        if (m.weights.size() / 4 != nv || m.joints.size() / 4 != nv) return;   /* 顶点集不完整时跳过 */
        if (m.bind_pos.size() != m.raw_pos.size() || m.bind_nrm.size() != m.raw_nrm.size())
                return;   /* 未备份绑定位姿则跳过 */
        const GltfSkin &skin = m.skins[0];
        size_t nj = skin.joints.size();
        if (nj == 0) return;

        /* 一次性诊断：绑定位姿 vs 蒙皮结果 的包围盒（定位掉脚/数值爆炸） */
        static bool diag_done = false;
        if (!diag_done) {
                diag_done = true;
                float bmin[3] = {1e9f,1e9f,1e9f}, bmax[3] = {-1e9f,-1e9f,-1e9f};
                for (size_t v = 0; v < nv; v++) {
                        for (int c = 0; c < 3; c++) {
                                float x = m.bind_pos[v*3+c];
                                if (x < bmin[c]) bmin[c] = x; if (x > bmax[c]) bmax[c] = x;
                        }
                }
                printf("[diag] bind AABB min(%.3f %.3f %.3f) max(%.3f %.3f %.3f)\n",
                       bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2]);
        }

        m.joint_mat.assign(nj * 16, 0.0f);
        for (size_t j = 0; j < nj; j++) {
                int node = skin.joints[j];
                if (node < 0 || node >= (int)m.nodes.size()) { identity_matrix(&m.joint_mat[j * 16]); continue; }
                const float *ng = &m.node_global[node * 16];
                const float *ib = skin.inv_bind.size() >= (j + 1) * 16 ? &skin.inv_bind[j * 16] : nullptr;
                float ib_ident[16];
                identity_matrix(ib_ident);
                multiply_matrix(ng, ib ? ib : ib_ident, &m.joint_mat[j * 16]);
        }

        for (size_t v = 0; v < nv; v++) {
                size_t off = v * 3;
                float px = 0, py = 0, pz = 0, nx = 0, ny = 0, nz = 0;
                bool moved = false;
                for (int q = 0; q < 4; q++) {
                        float w = m.weights[v * 4 + q];
                        if (w == 0.0f) continue;
                        int j = m.joints[v * 4 + q];
                        if (j < 0 || j >= (int)nj) continue;
                        moved = true;
                        const float *jm = &m.joint_mat[j * 16];
                        float bx = m.bind_pos[off], by = m.bind_pos[off + 1], bz = m.bind_pos[off + 2];
                        float bnx = m.bind_nrm[off], bny = m.bind_nrm[off + 1], bnz = m.bind_nrm[off + 2];
                        px += w * (jm[0] * bx + jm[4] * by + jm[8] * bz + jm[12]);
                        py += w * (jm[1] * bx + jm[5] * by + jm[9] * bz + jm[13]);
                        pz += w * (jm[2] * bx + jm[6] * by + jm[10] * bz + jm[14]);
                        nx += w * (jm[0] * bnx + jm[4] * bny + jm[8] * bnz);
                        ny += w * (jm[1] * bnx + jm[5] * bny + jm[9] * bnz);
                        nz += w * (jm[2] * bnx + jm[6] * bny + jm[10] * bnz);
                }
                if (!moved) continue;
                m.raw_pos[off] = px; m.raw_pos[off + 1] = py; m.raw_pos[off + 2] = pz;
                Vec3 nn = v3normalize(v3(nx, ny, nz));
                m.raw_nrm[off] = nn.x; m.raw_nrm[off + 1] = nn.y; m.raw_nrm[off + 2] = nn.z;
        }
        /* 一次性诊断：蒙皮后包围盒（与上方绑定位姿对比，定位掉脚/数值爆炸） */
        if (diag_done) {
                diag_done = false;
                float smin[3] = {1e9f,1e9f,1e9f}, smax[3] = {-1e9f,-1e9f,-1e9f};
                for (size_t v = 0; v < nv; v++) {
                        for (int c = 0; c < 3; c++) {
                                float x = m.raw_pos[v*3+c];
                                if (x < smin[c]) smin[c] = x; if (x > smax[c]) smax[c] = x;
                        }
                }
                printf("[diag] skin AABB min(%.3f %.3f %.3f) max(%.3f %.3f %.3f)\n",
                       smin[0],smin[1],smin[2], smax[0],smax[1],smax[2]);
        }
}

static void generate_sphere(int slices, int stacks, Model &m)
{
        for (int i = 0; i < stacks; i++) {
                float v0 = (float)i / stacks, v1 = (float)(i + 1) / stacks;
                float th0 = v0 * SDL_PI_F, th1 = v1 * SDL_PI_F;
                for (int j = 0; j < slices; j++) {
                        float u0 = (float)j / slices, u1 = (float)(j + 1) / slices;
                        float ph0 = u0 * 2.0f * SDL_PI_F, ph1 = u1 * 2.0f * SDL_PI_F;
                        Vec3 p[4];
                        for (int k = 0; k < 4; k++) {
                                float th = (k & 2) ? th1 : th0;
                                float ph = (k & 1) ? ph1 : ph0;
                                p[k] = v3(SDL_sinf(th) * SDL_cosf(ph),
                                          SDL_cosf(th),
                                          SDL_sinf(th) * SDL_sinf(ph));
                        }
                        /* 两个三角形：p0,p1,p2 和 p0,p2,p3（CCW 外侧） */
                        const Vec3 *tri[2][3] = {
                                {&p[0], &p[1], &p[2]},
                                {&p[0], &p[2], &p[3]}
                        };
                        for (int t = 0; t < 2; t++) {
                                for (int k = 0; k < 3; k++) {
                                        const Vec3 &v = *tri[t][k];
                                        m.raw_pos.push_back(v.x); m.raw_pos.push_back(v.y); m.raw_pos.push_back(v.z);
                                        m.raw_nrm.push_back(v.x); m.raw_nrm.push_back(v.y); m.raw_nrm.push_back(v.z);
                                }
                        }
                }
        }
}

/* 把 raw_pos/raw_nrm 打包成交错 pos+color（每次蒙皮后重新调用）。
 * 颜色：有材质色（m.colors）时用 材质色 × Lambert 假光照；否则法线伪彩色 + 假光照。
 * 若 raw_nrm 为空（如 glTF 无 NORMAL 属性），则按三角形叉积 + 质心朝向修正计算法线。 */
static void finalize_model(Model &m)
{
        static const Vec3 light = v3normalize(v3(0.35f, 0.8f, 0.5f));
        const bool has_nrm = m.raw_nrm.size() >= m.raw_pos.size();
        const bool has_mat = m.colors.size() >= m.raw_pos.size();
        size_t vcount = m.raw_pos.size() / 3;
        Vec3 center = v3(0, 0, 0);

        m.verts.clear();
        if (!has_nrm) {
                for (size_t i = 0; i < m.raw_pos.size(); i += 3)
                        center = v3add(center, v3(m.raw_pos[i], m.raw_pos[i + 1], m.raw_pos[i + 2]));
                if (vcount) center = v3scale(center, 1.0f / (float)vcount);
        }

        for (size_t i = 0; i + 8 < m.raw_pos.size(); i += 9) {
                Vec3 p0 = v3(m.raw_pos[i], m.raw_pos[i + 1], m.raw_pos[i + 2]);
                Vec3 p1 = v3(m.raw_pos[i + 3], m.raw_pos[i + 4], m.raw_pos[i + 5]);
                Vec3 p2 = v3(m.raw_pos[i + 6], m.raw_pos[i + 7], m.raw_pos[i + 8]);

                Vec3 n;
                if (has_nrm) {
                        n = v3normalize(v3(m.raw_nrm[i], m.raw_nrm[i + 1], m.raw_nrm[i + 2]));
                } else {
                        n = v3normalize(v3cross(v3sub(p1, p0), v3sub(p2, p0)));
                        Vec3 centroid = v3scale(v3add(v3add(p0, p1), p2), 1.0f / 3.0f);
                        if (v3dot(n, v3sub(centroid, center)) < 0.0f) n = v3scale(n, -1.0f);
                }

                float diffuse = SDL_max(0.0f, v3dot(n, light));
                float light_scale = 0.35f + 0.65f * diffuse;
                float base[3];
                if (has_mat) {
                        base[0] = m.colors[i];
                        base[1] = m.colors[i + 1];
                        base[2] = m.colors[i + 2];
                } else {
                        base[0] = 0.5f + 0.5f * SDL_fabsf(n.x);
                        base[1] = 0.5f + 0.5f * SDL_fabsf(n.y);
                        base[2] = 0.5f + 0.5f * SDL_fabsf(n.z);
                }

                const float tri[3][3] = {
                        {p0.x, p0.y, p0.z}, {p1.x, p1.y, p1.z}, {p2.x, p2.y, p2.z}
                };
                for (int k = 0; k < 3; k++) {
                        m.verts.push_back(tri[k][0]); m.verts.push_back(tri[k][1]); m.verts.push_back(tri[k][2]);
                        m.verts.push_back(base[0] * light_scale);
                        m.verts.push_back(base[1] * light_scale);
                        m.verts.push_back(base[2] * light_scale);
                }
        }
        m.index_count = (Uint32)(m.verts.size() / 6);
}

/* ==================== 着色器加载（按设备支持的格式选择） ==================== */

static SDL_GPUShader *load_shader(SDL_GPUDevice *device, bool is_vertex)
{
        SDL_GPUShaderCreateInfo ci;
        SDL_zero(ci);
        ci.num_uniform_buffers = is_vertex ? 1 : 0;

        SDL_GPUShaderFormat fmt = SDL_GetGPUShaderFormats(device);
        if (fmt & SDL_GPU_SHADERFORMAT_DXIL) {
                ci.format = SDL_GPU_SHADERFORMAT_DXIL;
                ci.code = is_vertex ? (const Uint8 *)cube_vert_dxil : (const Uint8 *)cube_frag_dxil;
                ci.code_size = is_vertex ? cube_vert_dxil_len : cube_frag_dxil_len;
        } else if (fmt & SDL_GPU_SHADERFORMAT_MSL) {
                ci.format = SDL_GPU_SHADERFORMAT_MSL;
                ci.code = is_vertex ? (const Uint8 *)cube_vert_msl : (const Uint8 *)cube_frag_msl;
                ci.code_size = is_vertex ? cube_vert_msl_len : cube_frag_msl_len;
        } else {
                ci.format = SDL_GPU_SHADERFORMAT_SPIRV;
                ci.code = is_vertex ? (const Uint8 *)cube_vert_spv : (const Uint8 *)cube_frag_spv;
                ci.code_size = is_vertex ? cube_vert_spv_len : cube_frag_spv_len;
        }
        ci.stage = is_vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
        return SDL_CreateGPUShader(device, &ci);
}

/* ==================== GPU 资源 ==================== */

struct GpuResources {
        SDL_GPUDevice *device = nullptr;
        SDL_GPUBuffer *buf_vertex = nullptr;
        SDL_GPUGraphicsPipeline *pipeline_fill = nullptr;
        SDL_GPUGraphicsPipeline *pipeline_line = nullptr;
        SDL_GPUTexture *tex_depth = nullptr;
        Uint32 depth_w = 0, depth_h = 0;
};

static SDL_GPUGraphicsPipeline *create_pipeline(SDL_GPUDevice *device,
                                               SDL_GPUShader *vs, SDL_GPUShader *fs,
                                               SDL_GPUTextureFormat swapchain_format,
                                               SDL_GPUFillMode fill_mode)
{
        SDL_GPUColorTargetDescription ct;
        SDL_zero(ct);
        ct.format = swapchain_format;

        SDL_GPUVertexBufferDescription vb_desc;
        SDL_zero(vb_desc);
        vb_desc.slot = 0;
        vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vb_desc.instance_step_rate = 0;
        vb_desc.pitch = 6 * sizeof(float);   /* pos3 + color3 */

        SDL_GPUVertexAttribute attrs[2];
        SDL_zero(attrs);
        attrs[0].location = 0;
        attrs[0].buffer_slot = 0;
        attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        attrs[0].offset = 0;
        attrs[1].location = 1;
        attrs[1].buffer_slot = 0;
        attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        attrs[1].offset = 3 * sizeof(float);

        SDL_GPUGraphicsPipelineCreateInfo pci;
        SDL_zero(pci);
        pci.vertex_shader = vs;
        pci.fragment_shader = fs;
        pci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pci.rasterizer_state.fill_mode = fill_mode;
        pci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
        pci.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
        pci.rasterizer_state.enable_depth_clip = true;
        pci.depth_stencil_state.enable_depth_test = true;
        pci.depth_stencil_state.enable_depth_write = true;
        pci.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
        pci.target_info.num_color_targets = 1;
        pci.target_info.color_target_descriptions = &ct;
        pci.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
        pci.target_info.has_depth_stencil_target = true;
        pci.vertex_input_state.num_vertex_buffers = 1;
        pci.vertex_input_state.vertex_buffer_descriptions = &vb_desc;
        pci.vertex_input_state.num_vertex_attributes = 2;
        pci.vertex_input_state.vertex_attributes = attrs;
        pci.props = 0;

        return SDL_CreateGPUGraphicsPipeline(device, &pci);
}

static SDL_GPUTexture *create_depth_texture(SDL_GPUDevice *device, Uint32 w, Uint32 h)
{
        SDL_GPUTextureCreateInfo ti;
        SDL_zero(ti);
        ti.type = SDL_GPU_TEXTURETYPE_2D;
        ti.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
        ti.width = w;
        ti.height = h;
        ti.layer_count_or_depth = 1;
        ti.num_levels = 1;
        ti.sample_count = SDL_GPU_SAMPLECOUNT_1;
        ti.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        ti.props = 0;
        return SDL_CreateGPUTexture(device, &ti);
}

/* 通过传输缓冲一次性把静态顶点数据上传到 GPU 缓冲 */
static SDL_GPUBuffer *upload_vertex_buffer(SDL_GPUDevice *device, const float *data, Uint32 bytes)
{
        SDL_GPUBufferCreateInfo bi;
        SDL_zero(bi);
        bi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bi.size = bytes;
        bi.props = 0;
        SDL_GPUBuffer *buf = SDL_CreateGPUBuffer(device, &bi);
        if (!buf) return nullptr;

        SDL_GPUTransferBufferCreateInfo tbi;
        SDL_zero(tbi);
        tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbi.size = bytes;
        tbi.props = 0;
        SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(device, &tbi);
        if (!transfer) { SDL_ReleaseGPUBuffer(device, buf); return nullptr; }

        void *map = SDL_MapGPUTransferBuffer(device, transfer, false);
        SDL_memcpy(map, data, bytes);
        SDL_UnmapGPUTransferBuffer(device, transfer);

        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(device);
        if (cmd) {
                SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);
                if (copy) {
                        SDL_GPUTransferBufferLocation src;
                        SDL_zero(src);
                        src.transfer_buffer = transfer;
                        src.offset = 0;
                        SDL_GPUBufferRegion dst;
                        SDL_zero(dst);
                        dst.buffer = buf;
                        dst.offset = 0;
                        dst.size = bytes;
                        SDL_UploadToGPUBuffer(copy, &src, &dst, false);
                        SDL_EndGPUCopyPass(copy);
                }
                SDL_SubmitGPUCommandBuffer(cmd);
        }
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        return buf;
}

/* 蒙皮后每帧覆盖上传顶点缓冲（大小不变），复用原缓冲避免反复创建 */
static bool update_vertex_buffer(SDL_GPUDevice *device, SDL_GPUBuffer *buf,
                                 const float *data, Uint32 bytes)
{
        SDL_GPUTransferBufferCreateInfo tbi;
        SDL_zero(tbi);
        tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbi.size = bytes;
        tbi.props = 0;
        SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(device, &tbi);
        if (!transfer) return false;

        void *map = SDL_MapGPUTransferBuffer(device, transfer, false);
        if (map) SDL_memcpy(map, data, bytes);
        SDL_UnmapGPUTransferBuffer(device, transfer);

        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(device);
        if (cmd) {
                SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);
                if (copy) {
                        SDL_GPUTransferBufferLocation src;
                        SDL_zero(src);
                        src.transfer_buffer = transfer;
                        src.offset = 0;
                        SDL_GPUBufferRegion dst;
                        SDL_zero(dst);
                        dst.buffer = buf;
                        dst.offset = 0;
                        dst.size = bytes;
                        SDL_UploadToGPUBuffer(copy, &src, &dst, false);
                        SDL_EndGPUCopyPass(copy);
                }
                SDL_SubmitGPUCommandBuffer(cmd);
        }
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        return true;
}

/* ==================== 主程序 ==================== */

/*
 * 创建 GPU 设备。后端尝试顺序（SDL 3.x GPU API 仅支持 d3d12 / vulkan / metal，
 * 本仓库 SDL 3.4 实际编译 d3d12+vulkan+metal，无 D3D11 后端）：
 *   1. 命令行 --driver 指定的后端
 *   2. SDL_GPU_DRIVER 环境变量指定的后端
 *   3. vulkan（本方案首选，Linux/Android/桌面均可用）
 *   4. d3d12（Windows 回退）
 *   5. NULL 交给 SDL 自动选择
 */
static SDL_GPUDevice *create_gpu_device(const char *preferred)
{
        const char *drivers[8];
        int n = 0;

        if (preferred && *preferred) drivers[n++] = preferred;
        const char *env = SDL_getenv("SDL_GPU_DRIVER");
        if (env && *env) drivers[n++] = env;
        drivers[n++] = "vulkan";   /* 优先 Vulkan */
        drivers[n++] = "d3d12";    /* 回退 D3D12 */
        drivers[n] = NULL;

        for (int i = 0; i < n; i++) {
                SDL_GPUDevice *dev = SDL_CreateGPUDevice(SUPPORTED_SHADER_FORMATS, true, drivers[i]);
                if (dev) {
                        printf("[gpu] backend requested='%s' -> created driver=%s\n",
                               drivers[i], SDL_GetGPUDeviceDriver(dev));
                        return dev;
                }
                printf("[warn] backend '%s' unavailable: %s\n", drivers[i], SDL_GetError());
        }
        return SDL_CreateGPUDevice(SUPPORTED_SHADER_FORMATS, true, NULL);
}

int main(int argc, char **argv)
{
        setvbuf(stdout, NULL, _IONBF, 0);   /* 禁用 stdout 缓冲，重定向/强杀时诊断不丢 */
        const char *model_path = "models/robot.glb";
        const char *preferred_driver = NULL;
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "--driver") == 0 && i + 1 < argc) {
                        preferred_driver = argv[++i];
                } else {
                        model_path = argv[i];
                }
        }
        SDL_Window *window = nullptr;
        GpuResources gpu;
        Model model;
        bool wireframe = false;
        bool auto_rotate = false;   /* 默认不自动旋转（空格切换） */
        float pitch = 45.0f, yaw = 30.0f, roll = 0.0f;   /* pitch=45° 俯视 */
        float cam_distance = 4.0f;
        float cam_target_y = 0.0f;   /* 注视点 y（模型空间，加载后按绑定位姿自动居中） */
        float model_scale = 0.3f;   /* 模型整体缩放（>1 放大，<1 缩小） */
        bool quit = false;
        Uint64 last_ticks, fps_ticks;
        Uint64 fps_frames = 0;

        setvbuf(stdout, NULL, _IONBF, 0);

        /* ---- 加载模型（.glb/.obj，失败先试源码 assets 目录，再兜底 UV 球体） ---- */
        {
                std::string err;
                /* 构建目录下模型复制为 models/，源码树里则是 assets/models/，
                 * 依工作目录不同按候选顺序尝试 */
                const char *candidates[] = { model_path, "assets/models/robot.glb" };
                const char *loaded = NULL;
                for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
                        if (load_model(candidates[i], model, err)) {
                                loaded = candidates[i];
                                break;
                        }
                }
                if (loaded) {
                        printf("[model] loaded '%s': triangles=%u\n", loaded,
                               (Uint32)(model.raw_pos.size() / 9));
                } else {
                        printf("[warn] load '%s' failed: %s\n", model_path, err.c_str());
                        printf("[info] fallback to generated UV sphere (32x16)\n");
                        generate_sphere(32, 16, model);
                }
                if (model.skinned)
                        printf("[model] skinned: %zu skin(s), %zu anim(s), %zu joint(s)\n",
                               model.skins.size(), model.anims.size(),
                               model.skins.empty() ? 0 : model.skins[0].joints.size());
                /* 默认播 Idle 动画（多动画 glTF 的第一个往往不是 Idle） */
                if (!model.anims.empty()) {
                        for (size_t a = 0; a < model.anims.size(); a++) {
                                const std::string &name = model.anim_names.empty()
                                        ? std::string("")
                                        : model.anim_names[a];
                                if (name.find("Idle") != std::string::npos ||
                                    name.find("idle") != std::string::npos) {
                                        model.anim_index = (int)a;
                                        break;
                                }
                        }
                        printf("[anim] default #%d\n", model.anim_index);
                }
                /* 备份绑定位姿：skin_model 每帧从 bind_pos 读输入（不会污染） */
                model.bind_pos = model.raw_pos;
                model.bind_nrm = model.raw_nrm;
                /* 注视点对齐绑定位姿垂直中心：让模型（含脚）完整居中在画面 */
                {
                        float ymin = 1e9f, ymax = -1e9f;
                        for (size_t v = 0; v + 2 < model.bind_pos.size(); v += 3) {
                                float y = model.bind_pos[v + 1];
                                if (y < ymin) ymin = y; if (y > ymax) ymax = y;
                        }
                        if (ymax > ymin) cam_target_y = (ymin + ymax) * 0.5f;
                        printf("[cam] target_y=%.2f distance=%.2f scale=%.2f\n",
                               cam_target_y, cam_distance, model_scale);
                }
                finalize_model(model);
                printf("[model] uploaded vertices=%u (pos3+color3 interleaved)\n", model.index_count);
        }

        /* ---- SDL / 窗口 / GPU 设备 ---- */
        if (!SDL_Init(SDL_INIT_VIDEO)) {
                printf("[error] SDL_Init: %s\n", SDL_GetError());
                return 1;
        }
        window = SDL_CreateWindow("model3d_gpu_test - SDL3 GPU 3D model",
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
        if (!window) {
                printf("[error] SDL_CreateWindow: %s\n", SDL_GetError());
                SDL_Quit();
                return 1;
        }
        gpu.device = create_gpu_device(preferred_driver);
        if (!gpu.device) {
                printf("[error] SDL_CreateGPUDevice: %s\n", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
        }
        if (!SDL_ClaimWindowForGPUDevice(gpu.device, window)) {
                printf("[error] SDL_ClaimWindowForGPUDevice: %s\n", SDL_GetError());
                SDL_DestroyGPUDevice(gpu.device);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
        }
        printf("[gpu] device created, driver=%s, shader formats=0x%x\n",
               SDL_GetGPUDeviceDriver(gpu.device), (unsigned)SDL_GetGPUShaderFormats(gpu.device));

        /* ---- 着色器 + 管线 ---- */
        SDL_GPUShader *vs = load_shader(gpu.device, true);
        SDL_GPUShader *fs = load_shader(gpu.device, false);
        if (!vs || !fs) {
                printf("[error] load shader: %s\n", SDL_GetError());
                quit = true; /* 走统一清理 */
        }
        if (!quit) {
                SDL_GPUTextureFormat fmt = SDL_GetGPUSwapchainTextureFormat(gpu.device, window);
                gpu.pipeline_fill = create_pipeline(gpu.device, vs, fs, fmt, SDL_GPU_FILLMODE_FILL);
                gpu.pipeline_line = create_pipeline(gpu.device, vs, fs, fmt, SDL_GPU_FILLMODE_LINE);
                if (!gpu.pipeline_fill || !gpu.pipeline_line) {
                        printf("[error] create pipeline: %s\n", SDL_GetError());
                        quit = true;
                }
                /* 管线创建后着色器即可释放（引用计数） */
                SDL_ReleaseGPUShader(gpu.device, vs);
                SDL_ReleaseGPUShader(gpu.device, fs);
        }

        /* ---- 顶点缓冲上传 ---- */
        if (!quit) {
                gpu.buf_vertex = upload_vertex_buffer(gpu.device, model.verts.data(),
                                                      (Uint32)(model.verts.size() * sizeof(float)));
                if (!gpu.buf_vertex) {
                        printf("[error] upload vertex buffer: %s\n", SDL_GetError());
                        quit = true;
                }
        }

        /* ---- 初始深度纹理 ---- */
        {
                int w, h;
                SDL_GetWindowSizeInPixels(window, &w, &h);
                gpu.tex_depth = create_depth_texture(gpu.device, (Uint32)w, (Uint32)h);
                gpu.depth_w = (Uint32)w;
                gpu.depth_h = (Uint32)h;
        }

        printf("[help] space=auto-rotate  arrows=pitch/yaw  A/D=roll  W/S=distance  F=wireframe  R=reset  N=next-anim\n");

        last_ticks = SDL_GetTicksNS();
        fps_ticks = last_ticks;

        /* ---- 主循环 ---- */
        while (!quit) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_EVENT_QUIT) quit = true;
                        else if (e.type == SDL_EVENT_KEY_DOWN) {
                                switch (e.key.key) {
                                case SDLK_ESCAPE: quit = true; break;
                                case SDLK_SPACE:  auto_rotate = !auto_rotate; break;
                                case SDLK_F:      wireframe = !wireframe; break;
                                case SDLK_N:
                                        if (!model.anims.empty()) {
                                                model.anim_index = (model.anim_index + 1) % (int)model.anims.size();
                                                model.anim_time = 0.0f;
                                                printf("[anim] now #%d '%s'\n", model.anim_index,
                                                       model.anim_names.empty() ? "" : model.anim_names[model.anim_index].c_str());
                                        }
                                        break;
                                case SDLK_R:
                                        pitch = 45.0f; yaw = 30.0f; roll = 0.0f; cam_distance = 3.5f;
                                        break;
                                default: break;
                                }
                        }
                }

                const bool *keys = SDL_GetKeyboardState(NULL);
                if (keys[SDL_SCANCODE_UP])    pitch += 3.0f;
                if (keys[SDL_SCANCODE_DOWN])  pitch -= 3.0f;
                if (keys[SDL_SCANCODE_LEFT])  yaw   += 3.0f;
                if (keys[SDL_SCANCODE_RIGHT]) yaw   -= 3.0f;
                if (keys[SDL_SCANCODE_A])     roll  += 3.0f;
                if (keys[SDL_SCANCODE_D])     roll  -= 3.0f;
                if (keys[SDL_SCANCODE_W])     cam_distance = SDL_max(0.8f, cam_distance - 0.05f);
                if (keys[SDL_SCANCODE_S])     cam_distance = SDL_min(20.0f, cam_distance + 0.05f);
                if (auto_rotate) { yaw += 1.2f; pitch += 0.4f; }
                if (yaw >= 360.0f) yaw -= 360.0f;
                if (pitch >= 360.0f) pitch -= 360.0f;

                /* ---- 动画 + CPU 蒙皮（skinned 模型每帧重打包并上传） ---- */
                if (model.skinned) {
                        Uint64 now_ns = SDL_GetTicksNS();
                        float dt = (float)((double)(now_ns - last_ticks) / 1000000000.0);
                        if (dt > 0.1f) dt = 0.1f;   /* 窗口拖动等长暂停后防跳变 */
                        last_ticks = now_ns;
                        update_animation(model, dt);
                        recompute_globals(model);
                        skin_model(model);
                        finalize_model(model);
                        if (!update_vertex_buffer(gpu.device, gpu.buf_vertex, model.verts.data(),
                                                  (Uint32)(model.verts.size() * sizeof(float))))
                                printf("[warn] skin upload failed: %s\n", SDL_GetError());
                } else {
                        last_ticks = SDL_GetTicksNS();
                }

                /* ---- MVP ---- */
                float m_rotX[16], m_rotY[16], m_rotZ[16], m_model[16], m_tmp[16];
                float m_scale[16];
                float m_proj[16], m_mvp[16];
                rotate_matrix(pitch, 1.0f, 0.0f, 0.0f, m_rotX);
                rotate_matrix(yaw,   0.0f, 1.0f, 0.0f, m_rotY);
                rotate_matrix(roll,  0.0f, 0.0f, 1.0f, m_rotZ);
                multiply_matrix(m_rotY, m_rotX, m_tmp);
                multiply_matrix(m_rotZ, m_tmp, m_model);
                /* 整体缩放模型（S*R，平移在缩放之外） */
                scale_matrix(model_scale, model_scale, model_scale, m_scale);
                multiply_matrix(m_scale, m_model, m_tmp);
                for (int i = 0; i < 16; i++) m_model[i] = m_tmp[i];
                m_model[13] -= cam_target_y * model_scale;   /* 注视点对齐模型垂直中心（脚完整可见） */
                m_model[14] -= cam_distance;   /* 列主序：第 14 项 = 平移 z */

                SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu.device);
                SDL_GPUTexture *swapchain = nullptr;
                Uint32 drawable_w = 0, drawable_h = 0;
                if (!cmd) { printf("[error] acquire cmd: %s\n", SDL_GetError()); break; }
                if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain,
                                                           &drawable_w, &drawable_h)) {
                        printf("[error] acquire swapchain: %s\n", SDL_GetError());
                        SDL_CancelGPUCommandBuffer(cmd);
                        break;
                }
                if (swapchain == nullptr) {   /* 窗口最小化等，跳过本帧 */
                        SDL_CancelGPUCommandBuffer(cmd);
                        continue;
                }

                /* 窗口尺寸变化时重建深度纹理 */
                if (drawable_w != gpu.depth_w || drawable_h != gpu.depth_h) {
                        SDL_ReleaseGPUTexture(gpu.device, gpu.tex_depth);
                        gpu.tex_depth = create_depth_texture(gpu.device, drawable_w, drawable_h);
                        gpu.depth_w = drawable_w;
                        gpu.depth_h = drawable_h;
                }

                float aspect = drawable_h ? (float)drawable_w / (float)drawable_h : 1.0f;
                perspective_matrix(45.0f, aspect, 0.01f, 100.0f, m_proj);
                multiply_matrix(m_proj, m_model, m_mvp);

                SDL_PushGPUVertexUniformData(cmd, 0, m_mvp, sizeof(m_mvp));

                SDL_GPUColorTargetInfo color_target;
                SDL_zero(color_target);
                color_target.clear_color.r = 0.12f;
                color_target.clear_color.g = 0.12f;
                color_target.clear_color.b = 0.14f;
                color_target.clear_color.a = 1.0f;
                color_target.load_op = SDL_GPU_LOADOP_CLEAR;
                color_target.store_op = SDL_GPU_STOREOP_STORE;
                color_target.texture = swapchain;
                color_target.cycle = true;

                SDL_GPUDepthStencilTargetInfo depth_target;
                SDL_zero(depth_target);
                depth_target.clear_depth = 1.0f;
                depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
                depth_target.store_op = SDL_GPU_STOREOP_DONT_CARE;
                depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
                depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
                depth_target.texture = gpu.tex_depth;
                depth_target.cycle = true;

                SDL_GPUBufferBinding binding;
                SDL_zero(binding);
                binding.buffer = gpu.buf_vertex;
                binding.offset = 0;

                SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, &depth_target);
                if (pass) {
                        SDL_BindGPUGraphicsPipeline(pass, wireframe ? gpu.pipeline_line : gpu.pipeline_fill);
                        SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
                        SDL_DrawGPUPrimitives(pass, model.index_count, 1, 0, 0);
                        SDL_EndGPURenderPass(pass);
                }
                SDL_SubmitGPUCommandBuffer(cmd);

                /* FPS 统计 */
                fps_frames++;
                Uint64 now = SDL_GetTicksNS();
                if (now - fps_ticks >= 500000000ULL) {
                        float fps = (float)((double)fps_frames / ((double)(now - fps_ticks) / 1000000000.0));
                        printf("[frame] fps=%.1f triangles=%u mode=%s yaw=%.0f dist=%.2f\n",
                               fps, model.index_count / 3, wireframe ? "wire" : "fill", yaw, cam_distance);
                        fps_frames = 0;
                        fps_ticks = now;
                }
        }

        /* ---- 清理 ---- */
        SDL_ReleaseGPUTexture(gpu.device, gpu.tex_depth);
        SDL_ReleaseGPUBuffer(gpu.device, gpu.buf_vertex);
        SDL_ReleaseGPUGraphicsPipeline(gpu.device, gpu.pipeline_fill);
        SDL_ReleaseGPUGraphicsPipeline(gpu.device, gpu.pipeline_line);
        SDL_ReleaseWindowFromGPUDevice(gpu.device, window);
        SDL_DestroyGPUDevice(gpu.device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        printf("[model3d] test finished.\n");
        return 0;
}
