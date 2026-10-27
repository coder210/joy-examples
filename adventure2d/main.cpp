#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <joy/joy.h>
#include <cstring>
#include "context.h"
#include "asset_manager.h"
#include "scenes/main_scene.h"
#include "scenes/splash_scene.h"
#include "scenes/game_scene.h"
#include "scenes/settings_scene.h"
#include "scenes/online_scene.h"
#include "layers/menu_layer.h"

// 统一使用 ctx->mgr 管理场景
static Context* ctx;

// 解析 --mode=single|host|client|server（默认 single）
static GameMode parse_mode(int argc, char* argv[])
{
        for (int i = 1; i < argc; i++) {
                if (strncmp(argv[i], "--mode=", 7) == 0) {
                        const char* m = argv[i] + 7;
                        if (strcmp(m, "host") == 0)   return MODE_HOST;
                        if (strcmp(m, "client") == 0) return MODE_CLIENT;
                        if (strcmp(m, "server") == 0) return MODE_SERVER;
                        if (strcmp(m, "single") == 0) return MODE_SINGLE;
                }
        }
        return MODE_SINGLE;
}

// 进入游戏场景：先重置上下文（含目标模式），再直接替换到游戏场景。
// 运行时切换模式与反复进出均通过此处 reset，避免 ecs 单例重复注册/系统重复添加。
static void enter_game_scene(GameMode mode)
{
        reset_context_for_mode(ctx, mode);
        scene_manager_replace(ctx->mgr, game_scene_get_scene(game_scene_create(ctx)));
}

// 处理加载/联机场景的模式按钮事件（服务端分支也复用，避免 ESC 返回后无法再切换模式）
static void handle_mode_event(SDL_Event* e)
{
        if (!is_user_event(e, MODE_EVENT)) return;
        switch (get_event_code(e)) {
        case MODE_EVENT_SINGLE: enter_game_scene(MODE_SINGLE); break;
        case MODE_EVENT_HOST:   enter_game_scene(MODE_HOST); break;
        case MODE_EVENT_CLIENT: enter_game_scene(MODE_CLIENT); break;
        case MODE_EVENT_SERVER: enter_game_scene(MODE_SERVER); break;
        }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
        sys_init_netenv();

        if (!init_env()) {
                log_info("SDL init failed: %s", get_error());
                return SDL_APP_FAILURE;
        }

        ctx = new Context();
        ctx->mode = parse_mode(argc, argv);
        settings_load(&ctx->settings);
        log_info("settings loaded: ip=%s port=%d bgm=%f sfx=%f fullscreen=%d",
                ctx->settings.server_ip, ctx->settings.server_port,
                ctx->settings.bgm_volume, ctx->settings.sfx_volume,
                ctx->settings.fullscreen ? 1 : 0);

        game_timer_init(&ctx->game_timer);
        game_timer_set_target_fps(&ctx->game_timer, 60);

        AssetManager::Init();

        ctx->window = create_window("adventure2d", 480, 320);
        ctx->r2d = r2d_create(ctx->window);
        r2d_set_logical_presentation(ctx->r2d, 480, 320);

        // 设置窗口图标
        window_set_icon(ctx->window, "adventure2d/textures/logo.png");

        // 应用全屏设置
        if (ctx->settings.fullscreen) {
                window_set_fullscreen(ctx->window, true);
        }

        ctx->running = true;

        ctx->mgr = scene_manager_create();
        if (ctx->mode == MODE_SERVER) {
                // 独立服务端：直接进入游戏场景（无菜单）
                scene_manager_push(ctx->mgr, game_scene_get_scene(game_scene_create(ctx)));
                log_info("server start");
        } else {
                // 单机/Host/客户端：先进入闪屏画面，然后自动切换到主场景
                scene_manager_push(ctx->mgr, splash_scene_get_scene(splash_scene_create(ctx)));
        }

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void*, SDL_Event* e)
{
        if (should_close(e)) return SDL_APP_SUCCESS;

        // 独立服务端：只响应退出，允许 ESC 返回主场景（可用于重新选择模式）
        if (ctx->mode == MODE_SERVER) {
                scene_manager_handle_event(ctx->mgr, e);
                if (e->type == SDL_EVENT_KEY_DOWN && e->key.scancode == SDL_SCANCODE_ESCAPE) {
                        log_info("[server] ESC pressed, back to main");
                        scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                }
                else if (is_user_event(e, TRANSITION_EVENT)) {
                        if (ctx->pending_scene) {
                                scene_manager_replace(ctx->mgr, ctx->pending_scene);
                                ctx->pending_scene = NULL;
                        }
                }
                else if (is_user_event(e, MENU_EVENT)) {
                        switch (get_event_code(e)) {
                        case MENU_EVENT_START:    enter_game_scene(MODE_SINGLE); break;
                        case MENU_EVENT_ONLINE:   scene_manager_replace(ctx->mgr, online_scene_get_scene(online_scene_create(ctx))); break;
                        case MENU_EVENT_SETTINGS: scene_manager_replace(ctx->mgr, settings_scene_get_scene(settings_scene_create(ctx))); break;
                        case MENU_EVENT_EXIT:     push_quit_event(); break;
                        }
                }
                else if (is_user_event(e, ONLINE_EVENT)) {
                        switch (get_event_code(e)) {
                        case ONLINE_EVENT_HOST:   enter_game_scene(MODE_HOST); break;
                        case ONLINE_EVENT_CLIENT: enter_game_scene(MODE_CLIENT); break;
                        case ONLINE_EVENT_SERVER: enter_game_scene(MODE_SERVER); break;
                        case ONLINE_EVENT_BACK:   scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx))); break;
                        }
                }
                else if (is_user_event(e, SETTINGS_EVENT)) {
                        if (get_event_code(e) == SETTINGS_EVENT_BACK)
                                scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                }
                handle_mode_event(e);
                return SDL_APP_CONTINUE;
        }

        scene_manager_handle_event(ctx->mgr, e);

        if (e->type == SDL_EVENT_KEY_DOWN) {
                switch (e->key.scancode) {
                case SDL_SCANCODE_SPACE:
                case SDL_SCANCODE_RETURN:
                        { scene_p cur = scene_manager_get_current(ctx->mgr);
                        if (cur && strcmp(scene_get_name(cur), "Main") == 0) {
                                enter_game_scene(ctx->mode);
                        } }
                        break;
                case SDL_SCANCODE_ESCAPE:
                        scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                        break;
                case SDL_SCANCODE_R:
                        scene_manager_destroy(ctx->mgr);
                        ctx->mgr = scene_manager_create();
                        scene_manager_push(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                        break;
                }
        }
        else if (is_user_event(e, MENU_EVENT)) {
                switch (get_event_code(e)) {
                case MENU_EVENT_START:
                        enter_game_scene(MODE_SINGLE);
                        break;
                case MENU_EVENT_ONLINE:
                        scene_manager_replace(ctx->mgr, online_scene_get_scene(online_scene_create(ctx)));
                        break;
                case MENU_EVENT_SETTINGS:
                        scene_manager_replace(ctx->mgr, settings_scene_get_scene(settings_scene_create(ctx)));
                        break;
                case MENU_EVENT_EXIT:
                        push_quit_event();
                        break;
                }
        }
        else if (is_user_event(e, SETTINGS_EVENT)) {
                if (get_event_code(e) == SETTINGS_EVENT_BACK) {
                        scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                }
        }
        else if (is_user_event(e, ONLINE_EVENT)) {
                switch (get_event_code(e)) {
                case ONLINE_EVENT_HOST:   enter_game_scene(MODE_HOST); break;
                case ONLINE_EVENT_CLIENT: enter_game_scene(MODE_CLIENT); break;
                case ONLINE_EVENT_SERVER: enter_game_scene(MODE_SERVER); break;
                case ONLINE_EVENT_BACK:
                        scene_manager_replace(ctx->mgr, main_scene_get_scene(main_scene_create(ctx)));
                        break;
                }
        }
        else if (is_user_event(e, MODE_EVENT)) {
                handle_mode_event(e);
        }
        else if (is_user_event(e, TRANSITION_EVENT)) {
                if (ctx->pending_scene) {
                        scene_manager_replace(ctx->mgr, ctx->pending_scene);
                        ctx->pending_scene = NULL;
                }
        }

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void*)
{
        game_timer_update(&ctx->game_timer);
        float dt = game_timer_get_delta_time(&ctx->game_timer);
        scene_manager_update(ctx->mgr, dt);
        r2d_set_color(ctx->r2d, 255, 255, 255, 255);
        r2d_begin(ctx->r2d);
        scene_manager_render(ctx->mgr, ctx->r2d);
        r2d_end(ctx->r2d);
        return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void*, SDL_AppResult)
{
        scene_manager_destroy(ctx->mgr);
        r2d_destroy(ctx->r2d);
        destroy_window(ctx->window);
        delete ctx;
        sys_release_netenv();
}