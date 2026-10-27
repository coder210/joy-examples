#include <cstring>
#include <joy/joy.h>
#include "game_scene_common.h"
#include "game_scene.h"
#include "../context.h"
#include "../protocol.h"
#include "../layers/gameplay_controls_layer.h"
#include "../network/network_manager.h"
#include "../systems/spawn_enemy_system.h"
#include "../systems/snapshot_system.h"
#include "../components/global_state_component.h"   // 视口缩放（Z 热键）
#include "../components/enemy_component.h"          // 校验取集合需判定死亡敌人
#include "../components/health_component.h"
#include "../systems/map_def.h"                     // MAP_W/MAP_H（自动缩放计算）

// ============================================================
// 视口缩放"自动适配整图"取值（与 game_scene 初始化同一套公式）
// ============================================================
// 自动适配：以"窗口高度"为基准，使地图高度恰好铺满窗口（消除上下黑边）；
// 宽度随窗口比例自适应（可横向滚动或留侧边，均不影响需求）。lw 仅作占位，不用于缩放。
float side_auto_view_zoom(int lw, int lh)
{
    (void)lw;
    const float ppm = meters_to_pixels(1.0f);
    float z = (float)lh / ((float)MAP_H * ppm);   // 高度铺满 = 窗口高 / (地图高*每米像素)
    if (z < 0.1f) z = 0.1f;
    if (z > 4.0f) z = 4.0f;
    return z;
}

// ============================================================
// 本机玩家键盘回调（单机/客户端/Host 共用）
// ============================================================
void GameSceneCommon::playKeyPressed(const char* key, int scancode, bool repeat,
                      bool ctrl, bool shift, bool alt, void* arg)
{
    GameScene* self = (GameScene*)arg;
    (void)scancode; (void)repeat; (void)ctrl; (void)shift; (void)alt;
    int key_mask = 0;
    if (strcmp(key, "a") == 0) key_mask = INPUT_LEFT;
    else if (strcmp(key, "A") == 0) key_mask = INPUT_LEFT;
    else if (strcmp(key, "d") == 0) key_mask = INPUT_RIGHT;
    else if (strcmp(key, "D") == 0) key_mask = INPUT_RIGHT;
    else if (strcmp(key, "w") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "W") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "s") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "S") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "up") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "down") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "j") == 0 || strcmp(key, "J") == 0) key_mask = INPUT_ATTACK;
    else if (strcmp(key, "z") == 0 || strcmp(key, "Z") == 0) {
        // Z：循环视口缩放（调试）——自动适配 → 原生1.0 → 0.7 → 0.5 → 自动。
        //    只改相机/渲染参数（不进快照、不进 checksum），不影响帧同步。
        static const float kPresets[] = { 0.0f, 1.0f, 0.7f, 0.5f };
        static int s_idx = 0;
        s_idx = (s_idx + 1) % (int)(sizeof(kPresets) / sizeof(kPresets[0]));
        const float preset = kPresets[s_idx];
        auto& gs = self->ctx()->ecs.ctx().get<GlobalState>();
        if (preset <= 0.0f) {
            // 自动：保持哨兵 0，camera_follow_pass 每帧按当前窗口尺寸重算（适配全屏/窗口缩放）
            gs.view_zoom = 0.0f;
            int lw = 480, lh = 320;
            r2d_get_logical_presentation(self->ctx()->r2d, &lw, &lh, NULL);
            log_info("[view] view_zoom = auto (窗口高 %d → 约 %.3f)", lh, side_auto_view_zoom(lw, lh));
        } else {
            gs.view_zoom = preset;
            log_info("[view] view_zoom = %.3f (manual)", preset);
        }
        return;
    }
    if (key_mask) {
        game_runtime(self->ctx()).current_input_mask |= key_mask;
        if (key_mask & INPUT_DIR_MASK)
            game_runtime(self->ctx()).local_dir = input_to_dir(key_mask);
    }
}

void GameSceneCommon::playKeyReleased(const char* key, int scancode, void* arg)
{
    GameScene* self = (GameScene*)arg;
    (void)scancode;
    int key_mask = 0;
    if (strcmp(key, "a") == 0) key_mask = INPUT_LEFT;
    else if (strcmp(key, "A") == 0) key_mask = INPUT_LEFT;
    else if (strcmp(key, "d") == 0) key_mask = INPUT_RIGHT;
    else if (strcmp(key, "D") == 0) key_mask = INPUT_RIGHT;
    else if (strcmp(key, "w") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "W") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "s") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "S") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "up") == 0) key_mask = INPUT_UP;
    else if (strcmp(key, "down") == 0) key_mask = INPUT_DOWN;
    else if (strcmp(key, "j") == 0 || strcmp(key, "J") == 0) key_mask = INPUT_ATTACK;
    if (key_mask) game_runtime(self->ctx()).current_input_mask &= ~key_mask;
}

// ============================================================
// 操作层事件 + 键盘事件分发（单机/客户端/Host 共用）
// ============================================================
void GameSceneCommon::playableHandleEvent(GameScene* self, const void* ev)
{
    Context* ctx = self->ctx();
    if (is_user_event(ev, GAMEPLAY_CONTROL_EVENT)) {
        int new_mask = game_runtime(ctx).current_input_mask;
        switch (get_event_code(ev)) {
        case GAMEPLAY_CONTROL_LEFT:  new_mask |= INPUT_LEFT; break;
        case GAMEPLAY_CONTROL_RIGHT: new_mask |= INPUT_RIGHT; break;
        case GAMEPLAY_CONTROL_UP:    new_mask |= INPUT_UP; break;
        case GAMEPLAY_CONTROL_DOWN:  new_mask |= INPUT_DOWN; break;
        case GAMEPLAY_CONTROL_RELEASE_LEFT:  new_mask &= ~INPUT_LEFT; break;
        case GAMEPLAY_CONTROL_RELEASE_RIGHT: new_mask &= ~INPUT_RIGHT; break;
        case GAMEPLAY_CONTROL_RELEASE_UP:    new_mask &= ~INPUT_UP; break;
        case GAMEPLAY_CONTROL_RELEASE_DOWN:  new_mask &= ~INPUT_DOWN; break;
        case GAMEPLAY_CONTROL_ATTACK:
            game_runtime(ctx).attack_triggered = true;
            break;
        default:
            new_mask &= ~(INPUT_UP | INPUT_DOWN | INPUT_LEFT | INPUT_RIGHT);
            break;
        }
        if (new_mask != game_runtime(ctx).current_input_mask) {
            game_runtime(ctx).current_input_mask = new_mask;
        }
        return;
    }
    jevent_callbacks_t cbs = {};
    cbs.on_keypressed = GameSceneCommon::playKeyPressed;
    cbs.on_keyreleased = GameSceneCommon::playKeyReleased;
    dispatch_event(cbs, ev, self);
}

// ============================================================
// 权威端初始化（单机 / Host / 服务端）
// ============================================================
void GameSceneCommon::initAuthoritative(Context* ctx)
{
    add_system(ctx, OnStart, "spawn_enemy", spawn_enemy_system);
    add_system(ctx, PostUpdate, "snapshot", snapshot_system);
    auto& gs = ctx->ecs.ctx().get<GlobalState>();
    gs.random_seed = (int)(get_ticks() ^ ((uintptr_t)&gs & 0xFFFF));
    gs.random = lcg_random_create((uint64_t)(uint32_t)gs.random_seed);
}

// ============================================================
// 本机操作层 + 音频（单机 / 客户端 / Host）
// ============================================================
void GameSceneCommon::initControlsAndAudio(GameScene* self)
{
    Context* ctx = self->ctx();
    self->gcl = new GameplayControlsLayer(ctx);   // 纯 button 操作层，直接挂在 gs_ 上由 GameScene 桥接

    ctx->audio_dev = audio_open_device();
    ctx->step_sound = audio_shot_create("joy_side_template/sounds/grass-step.wav", ctx->audio_dev);
    audio_shot_set_gain(ctx->step_sound, ctx->settings.sfx_volume);
    ctx->bgm = audio_bgm_create("joy_side_template/sounds/bgm.wav", ctx->audio_dev);
    audio_bgm_set_gain(ctx->bgm, ctx->settings.bgm_volume);
}

// ============================================================
// 注入本地玩家（单机 / Host）
// ============================================================
void GameSceneCommon::injectCommonPlayer(GameScene* self)
{
    self->net_.InjectLocalJoin();
}

// ============================================================
// 音频清理（单机 / 客户端 / Host）
// ============================================================
void GameSceneCommon::destroyPlayerAudio(Context* ctx)
{
    if (ctx->step_sound) { audio_shot_destroy(ctx->step_sound); ctx->step_sound = NULL; }
    if (ctx->bgm) { audio_bgm_destroy(ctx->bgm); ctx->bgm = NULL; }
    if (ctx->audio_dev) { audio_close_device(ctx->audio_dev); ctx->audio_dev = 0; }
}

// ============================================================
// 帧同步校验实现（客户端 / 单机共用）：由各模式 verifyFrameSync override 调用
// ============================================================
void GameSceneCommon::verifyFrameSyncImpl(GameScene* self, int frame_id,
                            S2CPlayerInput* inputs, int in, const char* server_checksum)
{
    Context* ctx = self->ctx();
    std::vector<ChecksumEntry> entries;
    auto pv = ctx->ecs.view<IdComponent, LogicPositionComponent>();
    for (auto e : pv) {
        // 必须与服务端 NetworkManager::CollectInput 的取集合规则完全一致：
        // 死亡敌人不纳入校验。原因见 CollectInput 注释——尸体销毁以权威帧号为基准，
        // 中途加入的客户端会晚几帧销毁，该差异是良性的，故排除以免误报 mismatch。
        if (ctx->ecs.all_of<EnemyComponent>(e) && ctx->ecs.all_of<HealthComponent>(e)
            && ctx->ecs.get<HealthComponent>(e).dead) continue;

        auto& idc = pv.get<IdComponent>(e);
        auto& lpc = pv.get<LogicPositionComponent>(e);
        entries.push_back({ idc.id, lpc.x, lpc.y });
    }
    char local_checksum[64] = { 0 };
    compute_full_md5(entries, local_checksum, sizeof(local_checksum));

    std::string input_finger;
    for (int k = 0; k < in; k++) {
        char b[64];
        snprintf(b, sizeof(b), "%d:%d:%d:%d:%d:",
                inputs[k].conv, inputs[k].dir,
                (int)inputs[k].attack, (int)inputs[k].ranged,
                (int)inputs[k].ray);
        input_finger += b;
    }

    int mismatched = (strcmp(local_checksum, server_checksum) != 0) ? 1 : 0;
    net_session(ctx).checksum_records.push_back(
            { frame_id, "", input_finger, checksum_entries_string(entries) });
    auto& rec = net_session(ctx).checksum_records.back();
    memcpy(rec.checksum, local_checksum, sizeof(rec.checksum));
    if (mismatched) {
        printf("[CHECKSUM] frame=%d MISMATCH server=%s local=%s inputs=%s entities=%s\n",
                frame_id, server_checksum, local_checksum,
                input_finger.c_str(), checksum_entries_string(entries).c_str());
        fflush(stdout);
    }
}
