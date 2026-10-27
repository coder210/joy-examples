#include <map>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <joy/joy.h>
#include "game_scene.h"
#include "../game_mode.h"
#include "../protocol.h"
#include "../factory/animator_def_factory.h"
#include "../network/network_utils.h"
#include "../systems/init_tilemap_system.h"
#include "../systems/input_consume_join_system.h"
#include "../systems/input_consume_leave_system.h"
#include "../systems/input_consume_input_system.h"
#include "../systems/movement_system.h"
#include "../systems/cleanup_system.h"
#include "../systems/collision_system.h"
#include "../systems/ai_system.h"
#include "../systems/animator_update_system.h"
#include "../systems/animator_render_system.h"
#include "../systems/drawing_entity_system.h"
#include "../systems/interpolate_position_system.h"
#include "../systems/camera_follow_system.h"
#include "../layers/debug_layer.h"
#include "../asset_manager.h"

static GameModeInterface* g_mode = NULL;

static void on_load(scene_p s)
{
        game_scene_p self = (game_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;
        log_info("[game scene] start, mode=%d", (int)ctx->mode);

        // 选择模式接口
        switch (ctx->mode) {
        case MODE_SINGLE: g_mode = &g_single_mode; break;
        case MODE_CLIENT: g_mode = &g_client_mode; break;
        case MODE_HOST:   g_mode = &g_host_mode;   break;
        case MODE_SERVER: g_mode = &g_server_mode; break;
        }

        debug_layer_p debug_layer = create_debug_layer();
        scene_add_root_node(self->scene, debug_layer_get_node(debug_layer));

        // 注册全局单例
        ctx->ecs.ctx().emplace<NetComponent>();
        ctx->ecs.ctx().emplace<GlobalState>();

        // 注册公共 system
        add_system(ctx, OnStart,   "init_tilemap",       init_tilemap_system);
        add_system(ctx, PreUpdate, "input_consume_join", input_consume_join_system);
        add_system(ctx, PreUpdate, "input_consume_leave", input_consume_leave_system);
        add_system(ctx, PreUpdate, "input_consume_input", input_consume_input_system);
        add_system(ctx, OnUpdate,  "ai_system",          ai_system);
        add_system(ctx, OnUpdate,  "movement",           movement_system);
        add_system(ctx, OnUpdate,  "collision",          collision_system);
        add_system(ctx, OnUpdate,  "animator_update",    animator_update_system);
        add_system(ctx, OnUpdate,  "cleanup",            cleanup_system);

        // 注册动画定义（全局单例）
        create_warrior_anim_def(ctx->ecs, "adventure2d/textures/knights/troops/warrior/warrior_blue.png");
        create_goblin_anim_def(ctx->ecs, "adventure2d/textures/goblins/troops/torch/torch_blue.png");

        // 模式特有初始化（可能追加 OnStart 阶段 system，如 spawn_enemy）
        g_mode->init(self);

        // 执行 OnStart phase（init_tilemap + 追加的 spawn_enemy）
        run_phase(ctx, OnStart, 0);
}

static void on_handle_event(scene_p s, const void* ev)
{
        game_scene_p self = (game_scene_p)scene_get_userdata(s);
        if (g_mode) g_mode->handle_event(self, ev);
}

static void on_update(scene_p s, float dt)
{
        game_scene_p self = (game_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;

        // 网络层更新（模式特有）
        g_mode->network_update(self);

        // 固定步长逻辑 Tick（20Hz）
        ctx->accumulator += dt;
        while (ctx->accumulator >= ctx->FIXED_TIMESTEP) {
            ctx->accumulator -= ctx->FIXED_TIMESTEP;

            // 模式特有 tick：帧号分配、输入收集、命令打包/发送
            g_mode->tick(self);

            // 消费一条命令帧（模拟前先写 NetComponent）
            bool got_command = false;
            if (!ctx->command_queue.empty()) {
                std::string command_data = ctx->command_queue.front();
                ctx->command_queue.pop();

                int frame_id, jn = 0, ln = 0, in = 0;
                S2CPlayerJoin joins[32];
                S2CPlayerLeave leaves[32];
                S2CPlayerInput inputs[64];
                bool ok = unpack_command(command_data.c_str(), (int)command_data.size(),
                    &frame_id, joins, 32, &jn, leaves, 32, &ln, inputs, 64, &in);
                if (ok) {
                    got_command = true;
                    auto& cmd = ctx->ecs.ctx().get<NetComponent>();
                    cmd.frame_id = frame_id;
                    cmd.num_joins = jn;  memcpy(cmd.joins, joins, jn * sizeof(S2CPlayerJoin));
                    cmd.num_leaves = ln; memcpy(cmd.leaves, leaves, ln * sizeof(S2CPlayerLeave));
                    cmd.num_inputs = in; memcpy(cmd.inputs, inputs, in * sizeof(S2CPlayerInput));
                }
            }

            // 客户端等到服务端命令才推进模拟；单机/Host/服务端本地总是有命令
            bool simulate = true;
            if (ctx->mode == MODE_CLIENT) simulate = got_command;
            if (simulate) {
                run_phase(ctx, PreUpdate, ctx->FIXED_TIMESTEP);
                run_phase(ctx, OnUpdate,  ctx->FIXED_TIMESTEP);
                run_phase(ctx, PostUpdate, ctx->FIXED_TIMESTEP);
            }
        }

        // 视觉相关 system 用实时 dt 每帧更新（不放在固定步长内）
        interpolate_position_system(ctx->ecs, ctx, dt);
        camera_follow_system(ctx->ecs, ctx, dt);
}

static void on_render(scene_p s)
{
        game_scene_p self = (game_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;
        r2d_set_color(ctx->r2d, 50, 120, 50, 255);

        // ---- tilemap 背景 ----
        if (ctx->tilemap && ctx->tilemap_render) {
                auto& cam = ctx->ecs.ctx().get<GlobalState>().camera_position;
                float cam_x_px = meters_to_pixels(cam.x);
                float cam_y_px = meters_to_pixels(cam.y);
                jtilemap_render_all(ctx->tilemap_render, ctx->tilemap, cam_x_px, cam_y_px, get_ticks());
        }

        // ---- ECS 实体渲染（有动画的用动画，没有的用方块）----
        auto draw_view = ctx->ecs.view<IdComponent, LogicRectComponent, TransformComponent>();
        for (auto e : draw_view) {
                auto& rect = draw_view.get<LogicRectComponent>(e);
                auto& t = draw_view.get<TransformComponent>(e);
                if (ctx->ecs.all_of<anim_t, anim_params_t, ActionStateComponent>(e)) {
                        auto& anim = ctx->ecs.get<anim_t>(e);
                        auto& act = ctx->ecs.get<ActionStateComponent>(e);
                        animator_render_system(ctx, ctx->ecs, t, rect, anim, act, e);
                } else {
                        auto& id = draw_view.get<IdComponent>(e);
                        drawing_entity_system(ctx, e, id, rect, t);
                }
        }

        // 模式特有渲染（碰撞调试等）
        if (g_mode) g_mode->render(self);
}

static void on_destroy(scene_p s)
{
        game_scene_p self = (game_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;

        // 网络清理
        if (ctx->netclient) { netclient_destroy(ctx->netclient); ctx->netclient = NULL; }
        if (ctx->net_server) { netserver_destroy(ctx->net_server); ctx->net_server = NULL; }
        AssetManager::GetInstance()->tex_cache_clear();

        // 模式特有清理（音频等）
        if (g_mode) g_mode->destroy(self);

        if (ctx->tilemap_render) { jtilemap_render_destroy(ctx->tilemap_render); ctx->tilemap_render = NULL; }
        if (ctx->tilemap) { jtilemap_destroy(ctx->tilemap); ctx->tilemap = NULL; }
        delete self;
}

game_scene_p game_scene_create(Context* ctx)
{
        game_scene_p self = new game_scene();
        self->scene = scene_create("game");
        self->ctx = ctx;
        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_event_callback(self->scene, on_handle_event);
        scene_set_update_callback(self->scene, on_update);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);
        return self;
}

scene_p game_scene_get_scene(game_scene_p s)
{
        return s ? s->scene : NULL;
}