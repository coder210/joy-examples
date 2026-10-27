#ifndef CONTEXT_H
#define CONTEXT_H
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "entt.hpp"
#include <joy/joy.h>

#include "protocol.h"
#include "components/action_state_component.h"
#include "components/animator_def.h"
#include "components/animator_component.h"
#include "components/attack_ray_effect_component.h"
#include "components/ai_component.h"
#include "components/connection_component.h"
#include "components/enemy_component.h"
#include "components/global_state_component.h"
#include "components/id_component.h"
#include "components/local_only_component.h"
#include "components/logic_position_component.h"
#include "components/logic_rect_component.h"
#include "components/logic_velocity_component.h"
#include "components/net_component.h"
#include "components/player_component.h"
#include "components/player_event_input.h"
#include "components/player_state_input.h"
#include "components/sprite_sheet_component.h"
#include "components/transform_component.h"

// 状态快照（用于预测回滚）
struct state_snapshot {
        fp_t position_x, position_y;
};

// 用户设置（自动保存到 settings.cfg，切换模式/进出场景时保留）
struct Settings {
        char  server_ip[64] = "192.168.1.5";
        int   server_port = 30000;
        float bgm_volume = 0.5f;   // 背景音乐音量 0~1
        float sfx_volume = 1.0f;   // 音效音量 0~1
        bool  fullscreen = false;
};

// 保存设置到 settings.cfg（key=value 文本格式）
static void settings_save(const Settings* s)
{
        FILE* fp = fopen("settings.cfg", "w");
        if (!fp) return;
        fprintf(fp, "server_ip=%s\n", s->server_ip);
        fprintf(fp, "server_port=%d\n", s->server_port);
        fprintf(fp, "bgm_volume=%f\n", s->bgm_volume);
        fprintf(fp, "sfx_volume=%f\n", s->sfx_volume);
        fprintf(fp, "fullscreen=%d\n", s->fullscreen ? 1 : 0);
        fclose(fp);
}

// 从 settings.cfg 加载设置（文件不存在时保留默认值）
static void settings_load(Settings* s)
{
        FILE* fp = fopen("settings.cfg", "r");
        if (!fp) return;
        char key[64], value[128];
        while (fscanf(fp, " %63[^=]=%127s", key, value) == 2) {
                if (strcmp(key, "server_ip") == 0) {
                        snprintf(s->server_ip, sizeof(s->server_ip), "%s", value);
                } else if (strcmp(key, "server_port") == 0) {
                        s->server_port = atoi(value);
                } else if (strcmp(key, "bgm_volume") == 0) {
                        s->bgm_volume = (float)atof(value);
                } else if (strcmp(key, "sfx_volume") == 0) {
                        s->sfx_volume = (float)atof(value);
                } else if (strcmp(key, "fullscreen") == 0) {
                        s->fullscreen = (atoi(value) != 0);
                }
        }
        fclose(fp);
}

// 运行模式（单二进制，运行时通过 --mode 参数切换）
enum GameMode {
        MODE_SINGLE = 0,   // 单机：本地推逻辑，不连网络
        MODE_HOST,         // 房主：本地推逻辑 + 网络服务端广播
        MODE_CLIENT,       // 客户端：连服务端/Host，消费命令 + 渲染
        MODE_SERVER,       // 独立服务端：只推逻辑 + 转发，无本地玩家
};

// 用户自定义事件类型
#define TRANSITION_EVENT (0x6)

// 跨场景过渡事件码
enum transition_event_code {
        TRANSITION_SPLASH_DONE = 1,
        TRANSITION_LOADING_DONE = 2,
};

struct Context {
        GameMode mode = MODE_SINGLE;
        bool is_server = false;  // 是否权威端（单机/Host/服务端），供部分 system 分支使用

        Settings settings;   // 用户设置（切换模式/进出场景时保留）

        bool running;
        game_timer_t game_timer;
        jwindow_p window;
        jr2d_p r2d;
        scene_manager_p mgr;  // 场景管理器（由 main.cpp 创建后赋值）
        scene_p pending_scene;  // 过渡场景（由 splash/loading 场景设置，事件处理中执行替换）

        // ---- 网络 ----
        netclient_p netclient;
        netserver_p net_server;
        entt::registry ecs;
        entt::entity singleton_net = entt::null;

        // ---- tilemap ----
        jtilemap_p          tilemap = NULL;
        jtilemap_render_p   tilemap_render = NULL;

        // ---- 音频 ----
        uint32_t            audio_dev = 0;
        audio_shot_p        step_sound = NULL;
        audio_bgm_p         bgm = NULL;
        int local_dir = 0; // 本地玩家方向
        std::map<entt::entity, int>  remote_dirs;

        std::map<int, int> server_inputs;
        bool ready = false;
        int server_frameid = 0;
        int server_entity_id = 0;

        // 固定逻辑步
        uint64_t lastTime = get_ticks();

        int current_input_mask = 0;
        bool attack_triggered = false;          // 按钮 tap 攻击，发送一次后自动复位
        float heartbeatTimer = 0.0f;

        // 本地玩家 conv（客户端由服务器 loading 下发的 conv 赋值；单机/Host 固定为 1）
        uint32_t local_conv = 0;
        std::map<int, std::map<int, state_snapshot>> snapshots;        // 按 frame_id 递增存储的历史快照
        std::vector<int> pending_inputs;     // 已发送但未确认的输入

        // ---- 固定步长逻辑 ----
        float accumulator = 0.0f;
        float FIXED_TIMESTEP = 1.0f / 20.0f;

        // ---- 命令缓冲 ----
        std::queue<std::string> command_queue;

        // ========== 服务端 / Host 字段（命令收集与转发）==========
        std::vector<S2CPlayerJoin> player_joins;
        std::vector<S2CPlayerLeave> player_leaves;
        std::vector<S2CPlayerInput> player_inputs;
        std::map<int, std::string> commands;          // 以帧号为键的命令帧缓冲
        std::map<int, std::string> world_snapshots;   // 以帧号为键的世界快照

        S2CEntity entities[512];
        int num_entities = 0;

        // system 注册
        std::vector<struct SystemEntry> systems;
};

// ========== System 调度基础设施 ==========
enum Phase { OnStart, PreUpdate, OnUpdate, PostUpdate };

struct SystemEntry {
        std::string name;
        Phase phase;
        std::function<void(entt::registry&, Context*, float)> fn;
};

inline void add_system(Context* ctx, Phase phase, const char* name,
        void (*fn)(entt::registry&, Context*, float))
{
        ctx->systems.push_back({ name, phase, fn });
}

inline void run_phase(Context* ctx, Phase phase, float dt)
{
        for (auto& sys : ctx->systems) {
                if (sys.phase == phase) {
                        sys.fn(ctx->ecs, ctx, dt);
                }
        }
}

// 全局实体 id 分配：统一从 GlobalState.next_entity_id 取。
// 服务端/Host/单机为权威分配；客户端在 handle_loading 时把 server_entity_id 对齐。
static int get_entity_id(Context* ctx)
{
    auto& gs = ctx->ecs.ctx().get<GlobalState>();
    return gs.next_entity_id++;
}

// 运行时切换模式：重置所有瞬时状态（ECS/系统注册/网络/命令缓冲），
// 保证可反复进出游戏场景而不重复注册系统或重复 emplace 单例组件。
static void reset_context_for_mode(Context* ctx, GameMode mode)
{
    if (ctx->netclient) { netclient_destroy(ctx->netclient); ctx->netclient = NULL; }
    if (ctx->net_server) { netserver_destroy(ctx->net_server); ctx->net_server = NULL; }

    ctx->ecs = entt::registry();   // 全新 registry（含 ctx() 上下文，避免重复 emplace 报错）
    ctx->systems.clear();

    if (ctx->tilemap_render) { jtilemap_render_destroy(ctx->tilemap_render); ctx->tilemap_render = NULL; }
    if (ctx->tilemap) { jtilemap_destroy(ctx->tilemap); ctx->tilemap = NULL; }

    ctx->command_queue = std::queue<std::string>();
    ctx->commands.clear();
    ctx->world_snapshots.clear();
    ctx->player_joins.clear();
    ctx->player_leaves.clear();
    ctx->player_inputs.clear();
    ctx->server_inputs.clear();
    ctx->snapshots.clear();
    ctx->pending_inputs.clear();

    ctx->ready = false;
    ctx->server_frameid = 0;
    ctx->server_entity_id = 0;
    ctx->local_conv = 0;
    ctx->num_entities = 0;
    ctx->accumulator = 0.0f;
    ctx->current_input_mask = 0;
    ctx->attack_triggered = false;
    ctx->local_dir = 0;
    ctx->heartbeatTimer = 0.0f;

    ctx->mode = mode;
}

#endif