#ifndef ADVENTURE2D_NETWORK_UTILS_H
#define ADVENTURE2D_NETWORK_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "../context.h"
#include "../components/global_state_component.h"
#include "../components/ai_component.h"
#include "../components/animator_def.h"
#include "../systems/spawn_def.h"
#include "../components/connection_component.h"

// ================================================================
// 客户端工具
// ================================================================

// ========== 加载数据处理 ==========
static void handle_loading(Context *ctx, int conv, int frame_id, int eid, const S2CEntity* entities, int ec)
{
        log_info("handle_loading");
        ctx->ready = true;
        ctx->local_conv = conv;
        ctx->ecs.ctx().get<GlobalState>().camera_follow_conv = conv;
        ctx->server_frameid = frame_id;
        ctx->server_entity_id = eid;

        for (int i = 0; i < ec; i++) {
                auto& entity = entities[i];
                auto e = ctx->ecs.create();
                ctx->ecs.emplace<IdComponent>(e, entity.id);
                ctx->ecs.emplace<LogicRectComponent>(e, entity.width, entity.height);
                ctx->ecs.emplace<LogicPositionComponent>(e, entity.position_x, entity.position_y);
                ctx->ecs.emplace<TransformComponent>(e, fp_to_float(entity.position_x),
                                                   fp_to_float(entity.position_y), 0.0f, 1.0f, 1.0f);
                if (entity.type == S2C_TYPE_PLAYER) {
                        ctx->ecs.emplace<LogicVelocityComponent>(e, fp_from_float(0), fp_from_float(0));
                        ctx->ecs.emplace<PlayerComponent>(e, entity.player_conv);
                        if (ctx->ecs.ctx().contains<janim_warrior_def_t>()) {
                            ctx->ecs.emplace<AnimatorComponent>(e);
                            ctx->ecs.emplace<AnimParamsComponent>(e);
                            ctx->ecs.emplace<ActionStateComponent>(e);
                        }
                }
                else if (entity.type == S2C_TYPE_ENEMY) {
                        ctx->ecs.emplace<EnemyComponent>(e);
                        ctx->ecs.emplace<ActionStateComponent>(e);
                        if (ctx->ecs.ctx().contains<janim_goblin_def_t>()) {
                            ctx->ecs.emplace<AnimatorComponent>(e);
                            ctx->ecs.emplace<AnimParamsComponent>(e);
                        }
                        int si = (int)entity.ai_type;
                        if (si >= 0 && si < ENEMY_SPAWN_COUNT) {
                            auto& sd = ENEMY_SPAWNS[si];
                            ctx->ecs.emplace<AIComponent>(e,
                                fp_from_float(sd.x), fp_from_float(sd.y),
                                fp_from_float(sd.patrol_radius),
                                fp_from_float(sd.detect_radius),
                                fp_from_float(sd.attack_radius));
                        }
                }
        }

        std::map<int, state_snapshot> snapshots;
        auto pv = ctx->ecs.view<PlayerComponent, IdComponent, LogicRectComponent, LogicPositionComponent>();
        for (auto e : pv) {
                auto& id = pv.get<IdComponent>(e);
                auto& pos = pv.get<LogicPositionComponent>(e);
                snapshots.insert({ id.id, { pos.x, pos.y } });
        }
        ctx->snapshots.insert({ ctx->server_frameid, snapshots });

        std::string data = pack_c2s(CMD_PLAYER_JOIN, fp_from_float(3.2f), fp_from_float(2.3f), 0, 0);
        netclient_send(ctx->netclient, data.c_str(), (int)data.size());
}

// ========== 发送输入 ==========
static void send_input(Context* ctx)
{
    if (!ctx->netclient) return;
    int dir = ctx->current_input_mask & INPUT_DIR_MASK;
    int attack = (ctx->current_input_mask & INPUT_ATTACK) ? 1 : 0;
    if (ctx->attack_triggered) { attack = 1; ctx->attack_triggered = false; }
    if (dir || attack) {
        auto data = pack_c2s(CMD_PLAYER_INPUT, 0, 0, dir, attack);
        netclient_send(ctx->netclient, data.c_str(), (int)data.size());
    }
}

// ========== 客户端网络接收 ==========
static void client_network_receive(Context* ctx)
{
        net_message_t msg;
        if (netclient_poll_message(ctx->netclient, &msg)) {
            if (msg.type == NET_TYPE_CONNECTED) {
                auto d = pack_c2s(CMD_LOADING, 0, 0, 0, 0);
                netclient_send(ctx->netclient, d.c_str(), (int)d.size());
            }
            else if (msg.type == NET_TYPE_DISCONNECTED) {
                log_info("disconnected=%d", msg.conv);
            }
            else if (msg.type == NET_TYPE_MESSAGE) {
                jmsgpack_unpacked_t r;
                jmsgpack_unpacked_init(&r);
                size_t off = 0;
                int is_loading = 0;
                if (jmsgpack_unpack_next(&r, (const char*)msg.data, (int)msg.len, &off) >= 0
                    && r.data.type == JMSGPACK_OBJECT_ARRAY && r.data.via.array.size >= 2
                    && r.data.via.array.ptr[0].type == JMSGPACK_OBJECT_POSITIVE_INTEGER)
                    is_loading = (r.data.via.array.ptr[0].via.u64 == S2C_CMD_LOADING);
                jmsgpack_unpacked_destroy(&r);

                if (is_loading) {
                    int conv, frame_id, eid;
                    S2CEntity entities[512];
                    int ec = unpack_loading((const char*)msg.data, (int)msg.len, &conv, &frame_id, &eid, entities, 512);
                    if (ec >= 0)
                        handle_loading(ctx, conv, frame_id, eid, entities, ec);
                }
                else {
                    ctx->command_queue.push(std::string(msg.data, msg.len));
                }
            }
            if (msg.type == NET_TYPE_MESSAGE && msg.data)
                netmessage_free_data(&msg);
        }
}

// ================================================================
// 服务端工具
// ================================================================

static void handle_cmd_loading(Context* ctx, int conv)
{
        log_info("C2S_CMD_LOADING");
        auto view = ctx->ecs.view<ConnectionComponent>();
        entt::entity conn = entt::null;
        for (auto e : view) {
                if (ctx->ecs.get<ConnectionComponent>(e).conv == conv) {
                        conn = e;
                        break;
                }
        }
        if (conn == entt::null) {
                conn = ctx->ecs.create();
                ctx->ecs.emplace<ConnectionComponent>(conn, conv);
        }
        auto& comp = ctx->ecs.get<ConnectionComponent>(conn);
        comp.is_loaded = false;
        comp.frameid = ctx->ecs.ctx().get<GlobalState>().next_frame_id;
}

static void handle_cmd_player_join(Context* ctx, int conv, int64_t x, int64_t y)
{
        S2CPlayerJoin j = { conv, x, y };
        ctx->player_joins.push_back(j);
}

static void handle_cmd_player_leave(Context* ctx, int conv)
{
        S2CPlayerLeave l = { conv };
        ctx->player_leaves.push_back(l);
}

static void handle_cmd_player_input(Context* ctx, int conv, uint32_t seq, int dir, int attack)
{
        S2CPlayerInput i = { seq, conv, dir, (int8_t)attack };
        ctx->player_inputs.push_back(i);
}

static void handle_cmd_heartbeat(Context* ctx, int conv)
{
        uint64_t ms_ticks;
        utils_get_current_time(&ms_ticks);
        auto view = ctx->ecs.view<ConnectionComponent>();
        entt::entity conn = entt::null;
        for (auto e : view) {
                if (ctx->ecs.get<ConnectionComponent>(e).conv == conv) {
                        conn = e;
                        break;
                }
        }
        if (conn == entt::null) return;
        auto& comp = ctx->ecs.get<ConnectionComponent>(conn);
        comp.health = 10;
        comp.last_heartbeat = (uint32_t)ms_ticks;
}

// ========== 服务端网络接收 ==========
static void server_network_receive(Context* ctx)
{
        net_message_t msg;
        while (netserver_poll_message(ctx->net_server, &msg)) {
                if (msg.type == NET_TYPE_CONNECTED) {
                        log_info("connected=%d", msg.conv);
                }
                else if (msg.type == NET_TYPE_DISCONNECTED) {
                        log_info("disconnected=%d", msg.conv);
                        S2CPlayerLeave l = { msg.conv };
                        ctx->player_leaves.push_back(l);
                }
                else if (msg.type == NET_TYPE_MESSAGE) {
                        int cmd, input_dir = 0, input_attack = 0; int64_t join_x = 0, join_y = 0;
                        if (unpack_c2s((const char*)msg.data, (int)msg.len, &cmd, &join_x, &join_y, &input_dir, &input_attack)) {
                                switch (cmd) {
                                case CMD_LOADING:       handle_cmd_loading(ctx, msg.conv); break;
                                case CMD_PLAYER_JOIN:   handle_cmd_player_join(ctx, msg.conv, join_x, join_y); break;
                                case CMD_PLAYER_LEAVE:  handle_cmd_player_leave(ctx, msg.conv); break;
                                case CMD_PLAYER_INPUT:  handle_cmd_player_input(ctx, msg.conv, 0, input_dir, input_attack); break;
                                case CMD_PLAYER_HEART:  handle_cmd_heartbeat(ctx, msg.conv); break;
                                }
                        }
                }
                if (msg.type == NET_TYPE_MESSAGE && msg.data) {
                        netmessage_free_data(&msg);
                }
        }
}

// ========== 帧同步辅助 ==========
static int get_target_frame_id(int curr_frameid, int context_frameid)
{
        int diff = context_frameid - curr_frameid;
        if (diff >= 50)      return curr_frameid + 9;
        if (diff >= 30)      return curr_frameid;
        if (diff >= 10)      return curr_frameid + 4;
        if (diff > 2)        return curr_frameid + 2;
        return curr_frameid;
}

// ========== 收集输入 + 打包命令帧 ==========
static void collect_input(Context* ctx)
{
        int frame_id = ctx->ecs.ctx().get<GlobalState>().next_frame_id++;
        S2CPlayerJoin joins[32]; int num_joins = 0;
        for (auto& j : ctx->player_joins)
                if (num_joins < 32) joins[num_joins++] = j;
        S2CPlayerLeave leaves[32]; int num_leaves = 0;
        for (auto& l : ctx->player_leaves)
                if (num_leaves < 32) leaves[num_leaves++] = l;
        S2CPlayerInput inputs[64]; int num_inputs = 0;
        for (auto& inp : ctx->player_inputs)
                if (num_inputs < 64) inputs[num_inputs++] = inp;

        std::vector<std::pair<int, std::pair<fp_t, fp_t>>> player_positions;
        auto pos_view = ctx->ecs.view<IdComponent, PlayerComponent, LogicPositionComponent>();
        for (auto e : pos_view) {
                auto& idc = pos_view.get<IdComponent>(e);
                auto& lpc = pos_view.get<LogicPositionComponent>(e);
                player_positions.push_back({ idc.id, { lpc.x, lpc.y } });
        }
        std::sort(player_positions.begin(), player_positions.end());

        std::string checksum_input;
        for (auto& player : player_positions) {
                char buf[64];
                int n = snprintf(buf, sizeof(buf), "%d:%lld:%lld,",
                        player.first, (long long)player.second.first,
                        (long long)player.second.second);
                checksum_input.append(buf, n);
        }
        char checksum[64] = { 0 };
        utils_md5x(checksum_input.c_str(), checksum_input.size(), checksum);

        auto command_data = pack_s2c(S2C_CMD_COMMAND, frame_id, joins, num_joins, leaves, num_leaves, inputs, num_inputs, checksum);
        ctx->commands[frame_id] = command_data;
        ctx->command_queue.push(command_data);

        ctx->player_joins.clear();
        ctx->player_leaves.clear();
        ctx->player_inputs.clear();
}

// ========== 转发命令帧给客户端 ==========
static void input_forwarding(Context* ctx)
{
        auto view = ctx->ecs.view<ConnectionComponent>();
        for (auto e : view) {
                auto& comp = view.get<ConnectionComponent>(e);
                if (comp.is_loaded) {
                        int taget_frameid = get_target_frame_id(comp.frameid,
                                ctx->ecs.ctx().get<GlobalState>().next_frame_id);
                        for (int i = comp.frameid; i < taget_frameid; i++) {
                                auto it = ctx->commands.find(i);
                                if (it != ctx->commands.end()) {
                                        netserver_send(ctx->net_server, comp.conv, it->second.c_str(), it->second.size());
                                }
                                else {
                                        log_error("Frame %d not found in commands, skip sending.", i);
                                }
                        }
                        comp.frameid = taget_frameid;
                }
                else {
                        auto it = ctx->world_snapshots.lower_bound(comp.frameid);
                        if (it == ctx->world_snapshots.end() || it->first > comp.frameid) {
                                if (it == ctx->world_snapshots.begin()) {
                                        log_info("WARNING: no snapshot cache found for frame %d (client %d), sending empty loading", comp.frameid, comp.conv);
                                        auto data = pack_s2c_loading(comp.conv, comp.frameid,
                                                ctx->ecs.ctx().get<GlobalState>().next_entity_id, nullptr, 0);
                                        netserver_send(ctx->net_server, comp.conv, data.c_str(), data.size());
                                        return;
                                }
                                --it;
                        }
                        S2CEntity entities[512];
                        int ec = 0, eid = 0;
                        ec = unpack_world(it->second.c_str(), (int)it->second.size(), &eid, entities, 512);
                        auto data = pack_s2c_loading(comp.conv, comp.frameid, eid, entities, ec);
                        netserver_send(ctx->net_server, comp.conv, data.c_str(), data.size());
                        comp.is_loaded = true;
                }
        }
}

#endif