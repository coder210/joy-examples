//// model_test.cpp — 用 SDL_GPU (通过 jr3d 库) 加载 glTF 模型并播放骨骼动画
//// 本文件是 joy_editor 当前唯一的活动 SDL 入口（其余 main*.cpp 入口均已注释）。
//// 骨骼蒙皮算法取自项目中已验证的 SkinnedModel 实现(main88888.cpp)。
//#define SDL_MAIN_USE_CALLBACKS
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <SDL3/SDL_log.h>
//
//#include <cstring>
//#include <cmath>
//#include <vector>
//#include <string>
//#include <filesystem>
//
//#include "joy/joy.h"
//#include "joy/jmath.h"
//#include "joy/external/cgltf.h"
//
//namespace fs = std::filesystem;
//
///* ============================================================
// * 骨骼蒙皮模型 (验证过的实现)
// * ============================================================ */
//struct SkinnedVertex {
//    vec3_t pos;
//    vec3_t normal;
//    vec2_t uv;
//    uint32_t joints[4];
//    float weights[4];
//};
//
//struct SkinnedMesh {
//    std::vector<SkinnedVertex> verts;
//    std::vector<uint32_t> indices;
//    render_mesh_p rmesh = nullptr;
//};
//
//struct SkinnedModel {
//    cgltf_data* data = nullptr;
//    cgltf_skin* skin = nullptr;
//    cgltf_node* rootNode = nullptr;
//    size_t jointCount = 0;
//
//    std::vector<SkinnedMesh> meshes;
//
//    std::vector<vec3_t> baseJointTranslation;
//    std::vector<vec4_t> baseJointRotation;
//    std::vector<vec3_t> baseJointScale;
//    bool baseHasMatrix = false;
//    mat44_t baseRootMatrix;
//
//    std::vector<mat44_t> jointLocal;
//    std::vector<mat44_t> jointGlobal;
//    std::vector<mat44_t> jointMatrix;
//    std::vector<mat44_t> inverseBind;
//
//    cgltf_animation* anim = nullptr;
//    float animDuration = 0.0f;
//
//    vec3_t modelOffset = {0,0,0};   // 脚底对齐 + XZ 居中
//    float modelScale = 1.0f;         // 归一化缩放(目标高度 ~1.2)
//};
//
///* jmath 没有的辅助: 四元数 TRS -> 列主序 mat44_t */
//static mat44_t mat44_from_trs(vec3_t t, vec4_t q, vec3_t s) {
//    float x=q.x, y=q.y, z=q.z, w=q.w;
//    float xx=x*x, yy=y*y, zz=z*z;
//    float xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
//    mat44_t m = mat44_identity();
//    m.m0  = (1.0f-2.0f*(yy+zz))*s.x; m.m4  = 2.0f*(xy-wz)*s.y;     m.m8  = 2.0f*(xz+wy)*s.z;
//    m.m1  = 2.0f*(xy+wz)*s.x;        m.m5  = (1.0f-2.0f*(xx+zz))*s.y; m.m9  = 2.0f*(yz-wx)*s.z;
//    m.m2  = 2.0f*(xz-wy)*s.x;        m.m6  = 2.0f*(yz+wx)*s.y;     m.m10 = (1.0f-2.0f*(xx+yy))*s.z;
//    m.m12 = t.x; m.m13 = t.y; m.m14 = t.z; m.m15 = 1.0f;
//    return m;
//}
//
//static bool loadSkinnedModel(const char* path, SkinnedModel& model) {
//    cgltf_options opt; memset(&opt, 0, sizeof(opt));
//    cgltf_data* data = nullptr;
//    cgltf_result res = cgltf_parse_file(&opt, path, &data);
//    if (res != cgltf_result_success) return false;
//    cgltf_load_buffers(&opt, data, path);
//    model.data = data;
//
//    // 找一个带 skin 的节点作为根
//    model.rootNode = nullptr;
//    for (cgltf_size i = 0; i < data->nodes_count; i++) {
//        if (data->nodes[i].skin) {
//            model.rootNode = &data->nodes[i];
//            model.skin = data->nodes[i].skin;
//            break;
//        }
//    }
//    if (!model.skin) return false;
//    model.jointCount = model.skin->joints_count;
//
//    // 逆绑定矩阵
//    model.inverseBind.resize(model.jointCount);
//    for (size_t i = 0; i < model.jointCount; i++) {
//        cgltf_accessor* acc = model.skin->inverse_bind_matrices;
//        if (acc) {
//            float m[16];
//            cgltf_accessor_read_float(acc, i, m, 16);
//            mat44_t mm;
//            mm.m0=m[0];  mm.m4=m[4];  mm.m8=m[8];   mm.m12=m[12];
//            mm.m1=m[1];  mm.m5=m[5];  mm.m9=m[9];   mm.m13=m[13];
//            mm.m2=m[2];  mm.m6=m[6];  mm.m10=m[10]; mm.m14=m[14];
//            mm.m3=m[3];  mm.m7=m[7];  mm.m11=m[11]; mm.m15=m[15];
//            model.inverseBind[i] = mm;
//        } else {
//            model.inverseBind[i] = mat44_identity();
//        }
//    }
//
//    // 记录静止姿态
//    model.baseHasMatrix = (model.rootNode->has_matrix != 0);
//    if (model.baseHasMatrix) {
//        const float* mt = model.rootNode->matrix;
//        model.baseRootMatrix.m0=mt[0];  model.baseRootMatrix.m4=mt[4];  model.baseRootMatrix.m8=mt[8];   model.baseRootMatrix.m12=mt[12];
//        model.baseRootMatrix.m1=mt[1];  model.baseRootMatrix.m5=mt[5];  model.baseRootMatrix.m9=mt[9];   model.baseRootMatrix.m13=mt[13];
//        model.baseRootMatrix.m2=mt[2];  model.baseRootMatrix.m6=mt[6];  model.baseRootMatrix.m10=mt[10]; model.baseRootMatrix.m14=mt[14];
//        model.baseRootMatrix.m3=mt[3];  model.baseRootMatrix.m7=mt[7];  model.baseRootMatrix.m11=mt[11]; model.baseRootMatrix.m15=mt[15];
//    }
//    model.baseJointTranslation.resize(model.jointCount);
//    model.baseJointRotation.resize(model.jointCount);
//    model.baseJointScale.resize(model.jointCount);
//    for (size_t i = 0; i < model.jointCount; i++) {
//        cgltf_node* jn = model.skin->joints[i];
//        model.baseJointTranslation[i] = { jn->translation[0], jn->translation[1], jn->translation[2] };
//        model.baseJointRotation[i] = { jn->rotation[0], jn->rotation[1], jn->rotation[2], jn->rotation[3] };
//        model.baseJointScale[i] = { jn->scale[0], jn->scale[1], jn->scale[2] };
//    }
//
//    model.jointLocal.resize(model.jointCount);
//    model.jointGlobal.resize(model.jointCount);
//    model.jointMatrix.resize(model.jointCount);
//
//    // 网格
//    model.meshes.resize(data->meshes_count);
//    for (cgltf_size mi = 0; mi < data->meshes_count; mi++) {
//        cgltf_mesh* cm = &data->meshes[mi];
//        SkinnedMesh& sm = model.meshes[mi];
//        for (cgltf_size p = 0; p < cm->primitives_count; p++) {
//            cgltf_primitive* prim = &cm->primitives[p];
//            const cgltf_accessor* pos = cgltf_find_accessor(prim, cgltf_attribute_type_position, 0);
//            const cgltf_accessor* nor = cgltf_find_accessor(prim, cgltf_attribute_type_normal, 0);
//            const cgltf_accessor* uv  = cgltf_find_accessor(prim, cgltf_attribute_type_texcoord, 0);
//            const cgltf_accessor* jnt = cgltf_find_accessor(prim, cgltf_attribute_type_joints, 0);
//            const cgltf_accessor* wgt = cgltf_find_accessor(prim, cgltf_attribute_type_weights, 0);
//            if (!pos) continue;
//            size_t vbase = sm.verts.size();
//            size_t vcount = pos->count;
//            sm.verts.resize(vbase + vcount);
//            for (cgltf_size i = 0; i < vcount; i++) {
//                SkinnedVertex& v = sm.verts[vbase + i];
//                float p3[3] = {0,0,0}; cgltf_accessor_read_float(pos, i, p3, 3);
//                v.pos = { p3[0], p3[1], p3[2] };
//                float n3[3] = {0,1,0}; if (nor) cgltf_accessor_read_float(nor, i, n3, 3);
//                v.normal = { n3[0], n3[1], n3[2] };
//                float t2[2] = {0,0}; if (uv) cgltf_accessor_read_float(uv, i, t2, 2);
//                v.uv = { t2[0], t2[1] };
//                unsigned int j4[4] = {0,0,0,0};
//                float w4[4] = {1,0,0,0};
//                if (jnt) cgltf_accessor_read_uint(jnt, i, j4, 4);
//                if (wgt) cgltf_accessor_read_float(wgt, i, w4, 4);
//                for (int k = 0; k < 4; k++) { v.joints[k] = j4[k]; v.weights[k] = w4[k]; }
//            }
//            if (prim->indices) {
//                size_t ibase = sm.indices.size();
//                sm.indices.resize(ibase + prim->indices->count);
//                for (cgltf_size i = 0; i < prim->indices->count; i++) {
//                    int vi = (int)cgltf_accessor_read_index(prim->indices, i);
//                    sm.indices[ibase + i] = (uint32_t)(vi + (int)vbase);
//                }
//            }
//        }
//    }
//
//    // 动画时长
//    if (data->animations_count > 0) {
//        model.anim = &data->animations[0];
//        float maxT = 0.0f;
//        for (cgltf_size c = 0; c < model.anim->channels_count; c++) {
//            cgltf_animation_channel* ch = &model.anim->channels[c];
//            if (ch->sampler && ch->sampler->input) {
//                cgltf_accessor* acc = ch->sampler->input;
//                float t = 0;
//                if (acc->count > 0) cgltf_accessor_read_float(acc, acc->count - 1, &t, 1);
//                if (t > maxT) maxT = t;
//            }
//        }
//        model.animDuration = maxT;
//    }
//    return true;
//}
//
//static void sampleChannel(cgltf_animation_channel* ch, float time,
//                          vec3_t& outT, vec4_t& outR, vec3_t& outS, bool& gotT, bool& gotR, bool& gotS) {
//    cgltf_animation_sampler* s = ch->sampler;
//    if (!s || !s->input || !s->output) return;
//    cgltf_accessor* inAcc = s->input;
//    cgltf_accessor* outAcc = s->output;
//    float t0=0, t1=0; cgltf_size i0=0, i1=0; bool found=false;
//    for (cgltf_size i = 0; i + 1 < inAcc->count; i++) {
//        float a=0, b=0;
//        cgltf_accessor_read_float(inAcc, i, &a, 1);
//        cgltf_accessor_read_float(inAcc, i+1, &b, 1);
//        if (time >= a && time <= b) { t0=a; t1=b; i0=i; i1=i+1; found=true; break; }
//    }
//    if (!found) {
//        if (time <= 0) { i0=i1=0; }
//        else { i0=i1=inAcc->count-1; }
//        t0 = t1 = 0;
//    }
//    float span = t1 - t0;
//    float k = (span > 1e-6f) ? (time - t0) / span : 0.0f;
//    int comps = (int)cgltf_num_components(outAcc->type);
//    std::vector<float> A(comps), B(comps), O(comps);
//    cgltf_accessor_read_float(outAcc, i0, A.data(), comps);
//    cgltf_accessor_read_float(outAcc, i1, B.data(), comps);
//    for (int c = 0; c < comps; c++) O[c] = A[c] + k * (B[c] - A[c]);
//
//    switch (ch->target_path) {
//        case cgltf_animation_path_type_translation:
//            outT = { O[0], O[1], O[2] }; gotT = true; break;
//        case cgltf_animation_path_type_scale:
//            outS = { O[0], O[1], O[2] }; gotS = true; break;
//        case cgltf_animation_path_type_rotation: {
//            vec4_t qa = { A[0],A[1],A[2],A[3] }, qb = { B[0],B[1],B[2],B[3] };
//            float d = qa.x*qb.x+qa.y*qb.y+qa.z*qb.z+qa.w*qb.w;
//            vec4_t qb2 = qb; float sgn = 1.0f;
//            if (d < 0.0f) { qb2 = { -qb.x,-qb.y,-qb.z,-qb.w }; sgn = -1.0f; }
//            vec4_t r;
//            r.x = qa.x + k*(qb2.x - qa.x); r.y = qa.y + k*(qb2.y - qa.y);
//            r.z = qa.z + k*(qb2.z - qa.z); r.w = qa.w + k*(qb2.w - qa.w);
//            float len = sqrtf(r.x*r.x+r.y*r.y+r.z*r.z+r.w*r.w);
//            if (len > 1e-8f) { r.x/=len; r.y/=len; r.z/=len; r.w/=len; }
//            outR = r; gotR = true; (void)sgn;
//        } break;
//        default: break;
//    }
//}
//
//static vec3_t g_dummyT = {0,0,0};
//static vec4_t g_dummyR = {0,0,0,1};
//static vec3_t g_dummyS = {1,1,1};
//
//static void updateNodeGlobal(cgltf_node* node, const mat44_t& parent, SkinnedModel& m) {
//    int jointIndex = -1;
//    for (size_t i = 0; i < m.jointCount; i++)
//        if (m.skin->joints[i] == node) { jointIndex = (int)i; break; }
//    mat44_t local;
//    if (jointIndex >= 0) local = m.jointLocal[jointIndex];
//    else {
//        if (node->has_matrix) {
//            const float* mt = node->matrix;
//            mat44_t mm;
//            mm.m0=mt[0];  mm.m4=mt[4];  mm.m8=mt[8];   mm.m12=mt[12];
//            mm.m1=mt[1];  mm.m5=mt[5];  mm.m9=mt[9];   mm.m13=mt[13];
//            mm.m2=mt[2];  mm.m6=mt[6];  mm.m10=mt[10]; mm.m14=mt[14];
//            mm.m3=mt[3];  mm.m7=mt[7];  mm.m11=mt[11]; mm.m15=mt[15];
//            local = mm;
//        } else {
//            vec3_t t = { node->translation[0],node->translation[1],node->translation[2] };
//            vec4_t q = { node->rotation[0],node->rotation[1],node->rotation[2],node->rotation[3] };
//            vec3_t s = { node->scale[0],node->scale[1],node->scale[2] };
//            local = mat44_from_trs(t, q, s);
//        }
//    }
//    mat44_t global = mat44_mul(parent, local);
//    if (jointIndex >= 0) {
//        m.jointGlobal[jointIndex] = global;
//        m.jointMatrix[jointIndex] = mat44_mul(global, m.inverseBind[jointIndex]);
//    }
//    for (cgltf_size i = 0; i < node->children_count; i++)
//        updateNodeGlobal(node->children[i], global, m);
//}
//
//static void restoreRest(SkinnedModel& m) {
//    if (m.baseHasMatrix) {
//        float* mt = m.rootNode->matrix;
//        mt[0]=m.baseRootMatrix.m0;  mt[4]=m.baseRootMatrix.m4;  mt[8]=m.baseRootMatrix.m8;   mt[12]=m.baseRootMatrix.m12;
//        mt[1]=m.baseRootMatrix.m1;  mt[5]=m.baseRootMatrix.m5;  mt[9]=m.baseRootMatrix.m9;   mt[13]=m.baseRootMatrix.m13;
//        mt[2]=m.baseRootMatrix.m2;  mt[6]=m.baseRootMatrix.m6;  mt[10]=m.baseRootMatrix.m10; mt[14]=m.baseRootMatrix.m14;
//        mt[3]=m.baseRootMatrix.m3;  mt[7]=m.baseRootMatrix.m7;  mt[11]=m.baseRootMatrix.m11; mt[15]=m.baseRootMatrix.m15;
//    }
//    for (size_t i = 0; i < m.jointCount; i++) {
//        cgltf_node* jn = m.skin->joints[i];
//        jn->translation[0] = m.baseJointTranslation[i].x;
//        jn->translation[1] = m.baseJointTranslation[i].y;
//        jn->translation[2] = m.baseJointTranslation[i].z;
//        jn->rotation[0] = m.baseJointRotation[i].x;
//        jn->rotation[1] = m.baseJointRotation[i].y;
//        jn->rotation[2] = m.baseJointRotation[i].z;
//        jn->rotation[3] = m.baseJointRotation[i].w;
//        jn->scale[0] = m.baseJointScale[i].x;
//        jn->scale[1] = m.baseJointScale[i].y;
//        jn->scale[2] = m.baseJointScale[i].z;
//    }
//}
//
//static void debugMin(SkinnedModel& m, vec3_t& outMin, vec3_t& outMax) {
//    outMin = { 1e9f,1e9f,1e9f }; outMax = { -1e9f,-1e9f,-1e9f };
//    for (SkinnedMesh& sm : m.meshes) {
//        for (SkinnedVertex& v : sm.verts) {
//            outMin.x = fminf(outMin.x, v.pos.x); outMin.y = fminf(outMin.y, v.pos.y); outMin.z = fminf(outMin.z, v.pos.z);
//            outMax.x = fmaxf(outMax.x, v.pos.x); outMax.y = fmaxf(outMax.y, v.pos.y); outMax.z = fmaxf(outMax.z, v.pos.z);
//        }
//    }
//}
//
//static void recomputeModelOffset(SkinnedModel& m) {
//    vec3_t mn, mx; debugMin(m, mn, mx);
//    float height = (mx.y - mn.y);
//    if (height < 1e-3f) height = 1.0f;
//    // 目标高度 1.2, 与 main88888 保持一致
//    m.modelScale = 1.2f / height;
//    float cx = (mn.x + mx.x) * 0.5f;
//    float cz = (mn.z + mx.z) * 0.5f;
//    // 蒙皮输出先减 mn.y 让脚底贴 y=0, 再 * scale, 再加 offset 让 XZ 居中
//    m.modelOffset = { -cx, 0.0f, -cz };
//}
//
//static void applyAnimation(SkinnedModel& m, float time) {
//    restoreRest(m);
//    if (!m.anim) return;
//    for (cgltf_size c = 0; c < m.anim->channels_count; c++) {
//        cgltf_animation_channel* ch = &m.anim->channels[c];
//        if (!ch->target_node) continue;
//        vec3_t t = g_dummyT; vec4_t r = g_dummyR; vec3_t s = g_dummyS;
//        bool gotT=false, gotR=false, gotS=false;
//        sampleChannel(ch, time, t, r, s, gotT, gotR, gotS);
//        cgltf_node* n = ch->target_node;
//        if (gotT) { n->translation[0]=t.x; n->translation[1]=t.y; n->translation[2]=t.z; }
//        if (gotR) { n->rotation[0]=r.x; n->rotation[1]=r.y; n->rotation[2]=r.z; n->rotation[3]=r.w; }
//        if (gotS) { n->scale[0]=s.x; n->scale[1]=s.y; n->scale[2]=s.z; }
//    }
//    // 重新合成关节 local
//    for (size_t i = 0; i < m.jointCount; i++) {
//        cgltf_node* jn = m.skin->joints[i];
//        vec3_t t = { jn->translation[0],jn->translation[1],jn->translation[2] };
//        vec4_t q = { jn->rotation[0],jn->rotation[1],jn->rotation[2],jn->rotation[3] };
//        vec3_t sc = { jn->scale[0],jn->scale[1],jn->scale[2] };
//        m.jointLocal[i] = mat44_from_trs(t, q, sc);
//    }
//    mat44_t identity = mat44_identity();
//    // 保险: 先把所有 jointGlobal 置为单位矩阵, 防止 FK 未覆盖某些 joint 时使用未初始化值导致顶点塌缩
//    for (size_t i = 0; i < m.jointCount; i++) m.jointGlobal[i] = mat44_identity();
//    // FK 必须覆盖所有 skin joints. 从带 skin 的节点递归不够稳健(它可能与 joints 树不一致),
//    // 改为从场景根 (&m.data->nodes[0]) 开始, 保证覆盖整棵节点树.
//    if (m.data && m.data->nodes_count > 0)
//        updateNodeGlobal(&m.data->nodes[0], identity, m);
//    else
//        updateNodeGlobal(m.rootNode, identity, m);
//}
//
//static void skinModel(SkinnedModel& m, jr3d_p r) {
//    recomputeModelOffset(m);
//    vec3_t mn, mx; debugMin(m, mn, mx);
//    float baseMinY = mn.y;
//    for (SkinnedMesh& sm : m.meshes) {
//        if (sm.verts.empty()) continue;
//        std::vector<float> vb;
//        vb.reserve(sm.verts.size() * 8);
//        for (SkinnedVertex& v : sm.verts) {
//            vec3_t sp = {0,0,0}, sn = {0,0,0};
//            // main88888 风格手写列主序乘法(避免任何 vec/ft 函数可能的副作用)
//            for (int k = 0; k < 4; k++) {
//                float w = v.weights[k];
//                if (w <= 0.0f) continue;
//                mat44_t M = mat44_identity();
//                if (m.skin && v.joints[k] < (unsigned int)m.jointCount)
//                    M = m.jointMatrix[v.joints[k]];
//                float x = v.pos.x, y = v.pos.y, z = v.pos.z;
//                sp.x += w * (M.m0*x + M.m4*y + M.m8*z + M.m12);
//                sp.y += w * (M.m1*x + M.m5*y + M.m9*z + M.m13);
//                sp.z += w * (M.m2*x + M.m6*y + M.m10*z + M.m14);
//                float nx = v.normal.x, ny = v.normal.y, nz = v.normal.z;
//                sn.x += w * (M.m0*nx + M.m4*ny + M.m8*nz);
//                sn.y += w * (M.m1*nx + M.m5*ny + M.m9*nz);
//                sn.z += w * (M.m2*nx + M.m6*ny + M.m10*nz);
//            }
//            // 脚底对齐 y=0(减原始 baseMinY), 再按 modelScale 归一化, 再加 XZ 居中偏移
//            float px = (sp.x) * m.modelScale + m.modelOffset.x;
//            float py = (sp.y - baseMinY) * m.modelScale + m.modelOffset.y;
//            float pz = (sp.z) * m.modelScale + m.modelOffset.z;
//            float nl = sqrtf(sn.x*sn.x + sn.y*sn.y + sn.z*sn.z);
//            if (nl > 1e-6f) { sn.x/=nl; sn.y/=nl; sn.z/=nl; }
//            vb.push_back(px); vb.push_back(py); vb.push_back(pz);
//            vb.push_back(sn.x); vb.push_back(sn.y); vb.push_back(sn.z);
//            vb.push_back(v.uv.x); vb.push_back(v.uv.y);
//        }
//        if (sm.rmesh) r3d_mesh_destroy(r, sm.rmesh);
//        sm.rmesh = r3d_mesh_custom_lit(r, vb.data(), (int)(vb.size()/8));
//    }
//}
//
///* ============================================================
// * 应用入口
// * ============================================================ */
//static jwindow_p gWin = nullptr;
//static jr3d_p gR3d = nullptr;
//static SkinnedModel gModel;
//static float gAnimTime = 0.0f;
//static Uint32 gLastTick = 0;
//
//static fs::path find_asset_root(const char* base) {
//    fs::path baseP(base);
//    fs::path candidates[] = {
//        baseP, baseP / "joy_editor", baseP.parent_path(),
//        baseP.parent_path() / "joy_editor", baseP.parent_path().parent_path(),
//    };
//    for (auto& c : candidates)
//        if (fs::exists(c / "shaders" / "vert.spv")) return c;
//    return baseP;
//}
//
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//    (void)argc; (void)argv;
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL", SDL_GetError(), nullptr);
//        return SDL_APP_FAILURE;
//    }
//    gWin = create_window("joy model test", 1280, 720);
//    if (!gWin) {
//        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "create_window", SDL_GetError(), nullptr);
//        return SDL_APP_FAILURE;
//    }
//
//    const char* basePath = SDL_GetBasePath();
//    if (!basePath) basePath = "./";
//    fs::path assetRoot = find_asset_root(basePath);
//    SDL_Log("assetRoot = %s", assetRoot.string().c_str());
//    SDL_free((void*)basePath);
//
//    fs::path shaderDir = assetRoot / "shaders";
//    gR3d = r3d_create(gWin, shaderDir.string().c_str());
//    if (!gR3d) {
//        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "r3d_create", "failed to create renderer", nullptr);
//        return SDL_APP_FAILURE;
//    }
//
//    fs::path modelPath = assetRoot / "models" / "robot.glb";
//    if (!loadSkinnedModel(modelPath.string().c_str(), gModel)) {
//        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "loadSkinnedModel",
//            ("load failed: " + modelPath.string()).c_str(), nullptr);
//        return SDL_APP_FAILURE;
//    }
//    SDL_Log("anim duration = %f s", gModel.animDuration);
//
//    r3d_font_init(gR3d, (assetRoot / "fonts" / "arial.ttf").string().c_str(), 28);
//
//    // 初始化光照
//    jr3d_light_block_t lb; memset(&lb, 0, sizeof(lb));
//    lb.lights[0].enabled = 1;
//    lb.lights[0].type = JR3D_LIGHT_DIRECTIONAL;
//    lb.lights[0].position[0] = 0.3f; lb.lights[0].position[1] = -1.0f; lb.lights[0].position[2] = 0.4f;
//    lb.lights[0].color[0] = 1.0f; lb.lights[0].color[1] = 1.0f; lb.lights[0].color[2] = 1.0f; lb.lights[0].color[3] = 1.0f;
//    lb.ambient[0] = 0.25f; lb.ambient[1] = 0.25f; lb.ambient[2] = 0.28f; lb.ambient[3] = 1.0f;
//    r3d_set_light_block(gR3d, &lb);
//
//    gLastTick = SDL_GetTicks();
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//    Uint32 now = SDL_GetTicks();
//    float dt = (now - gLastTick) / 1000.0f;
//    gLastTick = now;
//
//    if (gModel.animDuration > 0.0f) {
//        gAnimTime += dt;
//        if (gAnimTime > gModel.animDuration) gAnimTime -= gModel.animDuration;
//    }
//
//    applyAnimation(gModel, gAnimTime);
//    skinModel(gModel, gR3d);
//
//    if (!r3d_begin(gR3d)) return SDL_APP_CONTINUE;
//
//    // 固定相机: 略微俯视, 看向原点(模型应在这里)
//    vec3_t eye = { 2.5f, 2.0f, 2.5f };
//    vec3_t target = { 0.0f, 0.6f, 0.0f };
//    vec3_t up = { 0.0f, 1.0f, 0.0f };
//    r3d_camera_lookat(gR3d, eye, target, up, 60.0f);
//
//    // 参考物体 1: 地面网格(浅灰色平面, 验证相机/基础渲染)
//    static render_mesh_p floorMesh = r3d_mesh_plane_lit(gR3d, 10.0f, 10.0f, 1);
//    mat44_t floorM = mat44_identity();
//    r3d_draw_mesh_lit_color(gR3d, floorMesh, &floorM, { 60, 60, 60, 255 });
//
//    // 参考物体 2: 原点立方体(橙色, 验证坐标定位)
//    static render_mesh_p originCube = r3d_mesh_cube_lit(gR3d, 0.2f, 0.2f, 0.2f);
//    mat44_t cubeM = mat44_translate(0.0f, 0.6f, 0.0f);
//    r3d_draw_mesh_lit_color(gR3d, originCube, &cubeM, { 255, 140, 0, 255 });
//
//    jcolor_t color = { 205, 205, 205, 255 };
//    for (SkinnedMesh& sm : gModel.meshes) {
//        if (sm.rmesh) r3d_draw_mesh_lit_color(gR3d, sm.rmesh, &mat44_identity(), color);
//    }
//
//    int w = 0, h = 0;
//    r3d_get_window_size(gR3d, &w, &h);
//    char buf[128];
//    snprintf(buf, sizeof(buf), "t=%.2fs dur=%.2fs", gAnimTime, gModel.animDuration);
//    r3d_font_draw_2d(gR3d, r3d_mesh_quad(gR3d), buf, 20.0f, 20.0f, 24.0f, (float)w, (float)h);
//
//    r3d_end(gR3d);
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    (void)appstate;
//    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) return SDL_APP_SUCCESS;
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate; (void)result;
//    for (SkinnedMesh& sm : gModel.meshes)
//        if (sm.rmesh) { r3d_mesh_destroy(gR3d, sm.rmesh); sm.rmesh = nullptr; }
//    if (gR3d) { r3d_destroy(gR3d); gR3d = nullptr; }
//    if (gWin) { destroy_window(gWin); gWin = nullptr; }
//    if (gModel.data) { cgltf_free(gModel.data); gModel.data = nullptr; }
//    SDL_Quit();
//}
