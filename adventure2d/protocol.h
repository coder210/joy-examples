#pragma once
/* 冒险游戏网络协议（基于 jmsgpack）
 * Server ↔ Client 帧同步协议定义
 * 枚举、结构体、打包/解包工具函数 */
#include <joy/joy.h>
#include <cstring>
#include <cstdlib>

/* ================================ 常量 ================================ */
/* PIXELS_PER_METER 现由 joy/joy.h 统一提供，不再在此重复定义。 */

/* 输入位掩码 */
enum {
    INPUT_UP     = 1 << 0,
    INPUT_DOWN   = 1 << 1,
    INPUT_LEFT   = 1 << 2,
    INPUT_RIGHT  = 1 << 3,
    INPUT_ATTACK = 1 << 4,
    INPUT_DIR_MASK = INPUT_UP | INPUT_DOWN | INPUT_LEFT | INPUT_RIGHT,
};

/* C2S 命令 */
enum C2SCmd {
    CMD_UNKNOWN       = 0,
    CMD_PLAYER_JOIN   = 1,
    CMD_PLAYER_LEAVE  = 2,
    CMD_PLAYER_INPUT  = 3,
    CMD_PLAYER_HEART  = 4,
    CMD_LOADING       = 5,
};

/* S2C 命令 */
enum S2CCmd {
    S2C_CMD_NONE    = 0,
    S2C_CMD_LOADING = 1,
    S2C_CMD_COMMAND = 2,
};

/* S2C 实体类型 */
enum S2CType {
    S2C_TYPE_NORMAL = 0,
    S2C_TYPE_PLAYER = 1,
    S2C_TYPE_ENEMY  = 2,
};

/* ============================ 协议结构体 ============================ */
struct S2CPlayerJoin  { int32_t conv; int64_t position_x, position_y; };
struct S2CPlayerLeave { int32_t conv; };
struct S2CPlayerInput { uint32_t client_seq; int32_t conv, dir;  int8_t attack; };
struct S2CEntity {
    int32_t id, type;
    int64_t position_x, position_y, width, height, hp;
    int32_t player_conv;
    int64_t ai_type;       // AI 类型（SpawnDef 索引）
};

/* ======================== 辅助函数 ======================== */

/* 从 keycode 提取方向索引 (0=下, 1=左, 2=右, 3=上) */
static inline int input_to_dir(int keycode) {
    if (keycode & INPUT_DOWN)  return 0;
    if (keycode & INPUT_LEFT)  return 1;
    if (keycode & INPUT_RIGHT) return 2;
    if (keycode & INPUT_UP)    return 3;
    return 0;
}
static inline bool has_dir(int keycode) { return (keycode & INPUT_DIR_MASK) != 0; }

/* 从 jmsgpack 对象中提取有符号整数 */
static inline int64_t jmp_int(const jmsgpack_object_t* o) {
    if (jmsgpack_object_type(o) == JMSGPACK_OBJECT_POSITIVE_INTEGER) return (int64_t)jmsgpack_object_u64(o);
    if (jmsgpack_object_type(o) == JMSGPACK_OBJECT_NEGATIVE_INTEGER) return jmsgpack_object_i64(o);
    return 0;
}

/* ======================== msgpack 打包 ======================== */

/* 回写回调：写入 std::string */
static int mp_write_cb(void* data, const char* buf, size_t len) {
    ((std::string*)data)->append(buf, len);
    return 0;
}

/* --- 子对象打包 --- */
static inline void pack_entity(jmsgpack_packer_t* pk, const S2CEntity& e) {
    jmsgpack_pack_array(pk, 9);
    jmsgpack_pack_int(pk, e.id);    jmsgpack_pack_int(pk, e.type);
    jmsgpack_pack_int64(pk, e.position_x); jmsgpack_pack_int64(pk, e.position_y);
    jmsgpack_pack_int64(pk, e.width);      jmsgpack_pack_int64(pk, e.height);
    jmsgpack_pack_int64(pk, e.hp);  jmsgpack_pack_int(pk, e.player_conv);
    jmsgpack_pack_int64(pk, e.ai_type);
}
static inline void pack_join(jmsgpack_packer_t* pk, const S2CPlayerJoin& j) {
    jmsgpack_pack_array(pk, 3);
    jmsgpack_pack_int(pk, j.conv);
    jmsgpack_pack_int64(pk, j.position_x);
    jmsgpack_pack_int64(pk, j.position_y);
}
static inline void pack_leave(jmsgpack_packer_t* pk, const S2CPlayerLeave& l) {
    jmsgpack_pack_array(pk, 1);
    jmsgpack_pack_int(pk, l.conv);
}
static inline void pack_input(jmsgpack_packer_t* pk, const S2CPlayerInput& i) {
    jmsgpack_pack_array(pk, 3);
    jmsgpack_pack_int(pk, i.conv);
    jmsgpack_pack_int(pk, i.dir);
    jmsgpack_pack_int(pk, i.attack);
}

/* C2S 打包: [cmd, payload] */
static inline std::string pack_c2s(int cmd, int64_t join_x, int64_t join_y, int input_dir, int input_attack) {
    std::string out;
    jmsgpack_packer_t pk;
    jmsgpack_packer_init(&pk, &out, mp_write_cb);
    jmsgpack_pack_array(&pk, 2);
    jmsgpack_pack_int(&pk, cmd);
    if (cmd == CMD_PLAYER_JOIN) {
        jmsgpack_pack_array(&pk, 2);
        jmsgpack_pack_int64(&pk, join_x);
        jmsgpack_pack_int64(&pk, join_y);
    } else if (cmd == CMD_PLAYER_INPUT) {
        jmsgpack_pack_array(&pk, 2);
        jmsgpack_pack_int(&pk, input_dir);
        jmsgpack_pack_int(&pk, input_attack);
    } else {
        jmsgpack_pack_nil(&pk);
    }
    return out;
}

/* S2C 世界快照打包: [world_eid, [entity...]] */
static inline std::string pack_world(int entity_id, const S2CEntity* entities, int count) {
    std::string out;
    jmsgpack_packer_t pk;
    jmsgpack_packer_init(&pk, &out, mp_write_cb);
    jmsgpack_pack_array(&pk, 2);
    jmsgpack_pack_int(&pk, entity_id);
    jmsgpack_pack_array(&pk, count);
    for (int i = 0; i < count; i++)
        pack_entity(&pk, entities[i]);
    return out;
}

/* S2C loading 打包: [S2C_CMD_LOADING, [conv, frame_id, [world_eid, [entity...]]]] */
static inline std::string pack_s2c_loading(int32_t conv, int32_t frame_id, int32_t world_eid,
    const S2CEntity* entities, int ec)
{
    std::string out;
    jmsgpack_packer_t pk;
    jmsgpack_packer_init(&pk, &out, mp_write_cb);
    jmsgpack_pack_array(&pk, 2);
    jmsgpack_pack_int(&pk, S2C_CMD_LOADING);
    jmsgpack_pack_array(&pk, 3);
    jmsgpack_pack_int(&pk, conv);
    jmsgpack_pack_int(&pk, frame_id);
    jmsgpack_pack_array(&pk, 2);
    jmsgpack_pack_int(&pk, world_eid);
    jmsgpack_pack_array(&pk, ec);
    for (int i = 0; i < ec; i++)
        pack_entity(&pk, entities[i]);
    return out;
}

/* S2C command 打包（服务端用）: [S2C_CMD_COMMAND, [frame_id, joins, leaves, inputs, checksum]] */
static inline std::string pack_s2c(int16_t cmd, int32_t frame_id,
    const S2CPlayerJoin* joins, int jn,
    const S2CPlayerLeave* leaves, int ln,
    const S2CPlayerInput* inputs, int in,
    const char* checksum)
{
    std::string out;
    jmsgpack_packer_t pk;
    jmsgpack_packer_init(&pk, &out, mp_write_cb);
    jmsgpack_pack_array(&pk, 2);
    jmsgpack_pack_int(&pk, cmd);
    if (cmd == S2C_CMD_COMMAND) {
        jmsgpack_pack_array(&pk, 5);
        jmsgpack_pack_int(&pk, frame_id);
        jmsgpack_pack_array(&pk, jn);
        for (int i = 0; i < jn; i++) pack_join(&pk, joins[i]);
        jmsgpack_pack_array(&pk, ln);
        for (int i = 0; i < ln; i++) pack_leave(&pk, leaves[i]);
        jmsgpack_pack_array(&pk, in);
        for (int i = 0; i < in; i++) pack_input(&pk, inputs[i]);
        jmsgpack_pack_str(&pk, checksum ? strlen(checksum) : 0);
        if (checksum) jmsgpack_pack_str_body(&pk, checksum, strlen(checksum));
    } else {
        jmsgpack_pack_nil(&pk);
    }
    return out;
}

/* ======================== msgpack 解包 ======================== */

/* 解包单个实体 */
static inline void unpack_entity(const jmsgpack_object_t* o, S2CEntity* e) {
    if (!o || jmsgpack_object_type(o) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(o) < 9) return;
    e->id         = (int)jmp_int(jmsgpack_object_array_at(o, 0));
    e->type       = (int)jmp_int(jmsgpack_object_array_at(o, 1));
    e->position_x = jmp_int(jmsgpack_object_array_at(o, 2));
    e->position_y = jmp_int(jmsgpack_object_array_at(o, 3));
    e->width      = jmp_int(jmsgpack_object_array_at(o, 4));
    e->height     = jmp_int(jmsgpack_object_array_at(o, 5));
    e->hp         = jmp_int(jmsgpack_object_array_at(o, 6));
    e->player_conv = (int)jmp_int(jmsgpack_object_array_at(o, 7));
    e->ai_type    = jmp_int(jmsgpack_object_array_at(o, 8));
}

/* 解包 S2C 世界快照: [eid, [entity...]]，返回实体数 */
static inline int unpack_world(const char* data, int len, int* eid, S2CEntity* entities, int max_ec) {
    jmsgpack_unpacked_t r;
    jmsgpack_unpacked_init(&r);
    size_t off = 0;
    if (jmsgpack_unpack_next(&r, data, len, &off) < 0) { jmsgpack_unpacked_destroy(&r); return 0; }
    const jmsgpack_object_t* root = jmsgpack_unpacked_data(&r);
    if (jmsgpack_object_type(root) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(root) < 2) { jmsgpack_unpacked_destroy(&r); return 0; }
    *eid = (int)jmp_int(jmsgpack_object_array_at(root, 0));
    auto* arr = jmsgpack_object_array_at(root, 1);
    int n = (int)jmsgpack_object_array_size(arr);
    if (n > max_ec) n = max_ec;
    for (int i = 0; i < n; i++)
        unpack_entity(jmsgpack_object_array_at(arr, i), &entities[i]);
    jmsgpack_unpacked_destroy(&r);
    return n;
}

/* 解包 S2C loading 消息: 返回实体数，通过出参返回 conv/frame_id/eid */
static inline int unpack_loading(const char* data, int len,
    int* conv, int* frame_id, int* eid,
    S2CEntity* entities, int max_ec)
{
    jmsgpack_unpacked_t r;
    jmsgpack_unpacked_init(&r);
    size_t off = 0;
    if (jmsgpack_unpack_next(&r, data, len, &off) < 0) { jmsgpack_unpacked_destroy(&r); return -1; }
    const jmsgpack_object_t* root = jmsgpack_unpacked_data(&r);
    auto* pl = jmsgpack_object_array_at(root, 1);
    if (jmsgpack_object_type(pl) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(pl) < 3) { jmsgpack_unpacked_destroy(&r); return -1; }
    if (conv)     *conv     = (int)jmp_int(jmsgpack_object_array_at(pl, 0));
    if (frame_id) *frame_id = (int)jmp_int(jmsgpack_object_array_at(pl, 1));
    auto* w = jmsgpack_object_array_at(pl, 2);
    if (jmsgpack_object_type(w) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(w) < 2) { jmsgpack_unpacked_destroy(&r); return -1; }
    if (eid) *eid = (int)jmp_int(jmsgpack_object_array_at(w, 0));
    auto* arr = jmsgpack_object_array_at(w, 1);
    int n = (int)jmsgpack_object_array_size(arr);
    if (n > max_ec) n = max_ec;
    for (int i = 0; i < n; i++)
        unpack_entity(jmsgpack_object_array_at(arr, i), &entities[i]);
    jmsgpack_unpacked_destroy(&r);
    return n;
}

/* 解包 S2C command 消息（通用版，服务端/客户端均可用）
 * 返回 1 成功，0 失败。
 * 通过出参返回 frame_id / joins / leaves / inputs */
static inline int unpack_command(const char* data, int len,
    int* frame_id,
    S2CPlayerJoin* joins, int max_jn, int* jn,
    S2CPlayerLeave* leaves, int max_ln, int* ln,
    S2CPlayerInput* inputs, int max_in, int* in)
{
    if (jn)  memset(joins,  0, max_jn * sizeof(S2CPlayerJoin));
    if (ln)  memset(leaves, 0, max_ln * sizeof(S2CPlayerLeave));
    if (in)  memset(inputs, 0, max_in * sizeof(S2CPlayerInput));
    if (jn)  *jn = 0;
    if (ln)  *ln = 0;
    if (in)  *in = 0;

    jmsgpack_unpacked_t r;
    jmsgpack_unpacked_init(&r);
    size_t off = 0;
    if (jmsgpack_unpack_next(&r, data, len, &off) < 0) { jmsgpack_unpacked_destroy(&r); return 0; }
    const jmsgpack_object_t* root = jmsgpack_unpacked_data(&r);
    if (jmsgpack_object_type(root) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(root) < 2) { jmsgpack_unpacked_destroy(&r); return 0; }
    auto* pl = jmsgpack_object_array_at(root, 1);
    if (jmsgpack_object_type(pl) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(pl) < 5) { jmsgpack_unpacked_destroy(&r); return 0; }

    if (frame_id) *frame_id = (int)jmp_int(jmsgpack_object_array_at(pl, 0));

    auto read_arr = [&](int idx, auto* out, int max, int& cnt) {
        if (idx >= (int)jmsgpack_object_array_size(pl)) return;
        auto* a = jmsgpack_object_array_at(pl, idx);
        if (jmsgpack_object_type(a) != JMSGPACK_OBJECT_ARRAY) return;
        int n = (int)jmsgpack_object_array_size(a);
        if (n > max) n = max;
        cnt = n;
        for (int i = 0; i < n; i++) {
            auto* o = jmsgpack_object_array_at(a, i);
            auto& d = out[i];
            using T = std::decay_t<decltype(d)>;
            if constexpr (std::is_same_v<T, S2CPlayerJoin>) {
                if (jmsgpack_object_type(o) == JMSGPACK_OBJECT_ARRAY && jmsgpack_object_array_size(o) >= 3) {
                    d.conv = (int)jmp_int(jmsgpack_object_array_at(o, 0));
                    d.position_x = jmp_int(jmsgpack_object_array_at(o, 1));
                    d.position_y = jmp_int(jmsgpack_object_array_at(o, 2));
                }
            } else if constexpr (std::is_same_v<T, S2CPlayerLeave>) {
                if (jmsgpack_object_type(o) == JMSGPACK_OBJECT_ARRAY && jmsgpack_object_array_size(o) >= 1) {
                    d.conv = (int)jmp_int(jmsgpack_object_array_at(o, 0));
                }
            } else if constexpr (std::is_same_v<T, S2CPlayerInput>) {
                if (jmsgpack_object_type(o) == JMSGPACK_OBJECT_ARRAY && jmsgpack_object_array_size(o) >= 3) {
                    d.conv = (int)jmp_int(jmsgpack_object_array_at(o, 0));
                    d.dir = (int)jmp_int(jmsgpack_object_array_at(o, 1));
                    d.attack = (int8_t)jmp_int(jmsgpack_object_array_at(o, 2));
                }
            }
        }
    };

    read_arr(1, joins,  max_jn, *jn);
    read_arr(2, leaves, max_ln, *ln);
    read_arr(3, inputs, max_in, *in);

    jmsgpack_unpacked_destroy(&r);
    return 1;
}

/* 解包 C2S 消息: [cmd, payload]
 * 返回 1 成功，0 失败。
 * 通过出参返回 cmd / join_x, join_y / input_dir, input_attack */
static inline int unpack_c2s(const char* data, int len,
    int* cmd, int64_t* join_x, int64_t* join_y, int* input_dir, int* input_attack)
{
    jmsgpack_unpacked_t r;
    jmsgpack_unpacked_init(&r);
    size_t off = 0;
    if (jmsgpack_unpack_next(&r, data, len, &off) < 0) { jmsgpack_unpacked_destroy(&r); return 0; }
    const jmsgpack_object_t* root = jmsgpack_unpacked_data(&r);
    if (jmsgpack_object_type(root) != JMSGPACK_OBJECT_ARRAY || jmsgpack_object_array_size(root) < 2) { jmsgpack_unpacked_destroy(&r); return 0; }
    *cmd = (int)jmp_int(jmsgpack_object_array_at(root, 0));
    auto* payload = jmsgpack_object_array_at(root, 1);
    if (jmsgpack_object_type(payload) == JMSGPACK_OBJECT_ARRAY) {
        switch (*cmd) {
        case CMD_PLAYER_JOIN:
            if (jmsgpack_object_array_size(payload) >= 2) {
                *join_x = jmp_int(jmsgpack_object_array_at(payload, 0));
                *join_y = jmp_int(jmsgpack_object_array_at(payload, 1));
            }
            break;
        case CMD_PLAYER_INPUT:
            if (jmsgpack_object_array_size(payload) >= 2) {
                *input_dir = (int)jmp_int(jmsgpack_object_array_at(payload, 0));
                *input_attack = (int)jmp_int(jmsgpack_object_array_at(payload, 1));
            }
            break;
        }
    }
    jmsgpack_unpacked_destroy(&r);
    return 1;
}


