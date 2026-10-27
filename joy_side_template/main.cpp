#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <joy/joy.h>
#include <cstring>
#include <memory>
#include "context.h"
#include "asset_manager.h"
#include "scenes/scene.h"
#include "scenes/scene_manager.h"
#include "systems/character_catalog.h"
#include "systems/enemy_catalog.h"
#include "scenes/main_scene.h"
#include "scenes/splash_scene.h"
#include "scenes/loading_scene.h"
#include "scenes/game_scene.h"
#include "scenes/settings_scene.h"
#include "scenes/online_scene.h"
#include "scenes/character_select_scene.h"
#include "scenes/victory_scene.h"
#include "layers/menu_layer.h"
#include "passes/draw_fps_pass.h"


// 场景管理统一交给 C++ SceneManager（阶段1：内部转调 joy 的 scene_manager，后续阶段可脱离 C 库）
static Context* ctx;
static SceneManager* g_mgr = nullptr;

// 解析 --mode=single|host|client|server（默认 single）
static GameModeKind parse_mode(int argc, char* argv[])
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

// 进入游戏场景（通过加载场景过渡，支持运行时切换模式与反复进出）
static void enter_game_scene(GameModeKind mode)
{
        g_mgr->change(new LoadingScene(ctx, mode));
}

// 进入角色选择场景（单机/房主/客户端统一的流程入口）
static void enter_character_select(GameModeKind mode)
{
        g_mgr->change(new CharacterSelectScene(ctx, mode));
}

// 切换到胜利场景（game_scene 判定全清后推送 VICTORY_EVENT）
static void handle_victory_event()
{
        g_mgr->processPending();
}

// 处理加载场景的模式按钮事件（服务端分支也复用，避免 ESC 返回加载场景后无法再切换模式）
static void handle_mode_event(SDL_Event* e)
{
        if (!is_user_event(e, MODE_EVENT)) return;
        switch (get_event_code(e)) {
        case MODE_EVENT_SINGLE: 
                enter_game_scene(MODE_SINGLE);
                break;
        case MODE_EVENT_HOST:   
                enter_game_scene(MODE_HOST);
                break;
        case MODE_EVENT_CLIENT:
                enter_game_scene(MODE_CLIENT);
                break;
        case MODE_EVENT_SERVER: 
                enter_game_scene(MODE_SERVER);
                break;
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
        // 角色表必须在 AssetManager 之后加载（其路径经资源索引 config.characters 取得）
        g_characterCatalog.load();
        // 敌人表同理（路径经资源索引 config.enemies 取得）。
        // 必须在 map_load_level 之前：关卡解析敌人出生点时要按 key 反查种类 id。
        g_enemyCatalog.load();

        ctx->window = create_window("joy_side_template", 480, 320);
        ctx->r2d = r2d_create(ctx->window);
        r2d_set_logical_presentation_mode(ctx->r2d, 480, 320, JOY_LOGICAL_PRESENTATION_LETTERBOX);

        window_set_icon(ctx->window, AssetManager::kPathBrandingLogo);

        if (ctx->settings.fullscreen) {
                window_set_fullscreen(ctx->window, true);
        }

        ctx->running = true;

        g_mgr = new SceneManager();
        ctx->scene_mgr = g_mgr;   // 场景经此登记过渡目标（不拥有 SceneManager）
        if (ctx->mode == MODE_SERVER) {
                // 独立服务端：直接进入游戏场景（无菜单）
                g_mgr->push(new GameScene(ctx));
                log_info("server start");
        } else {
                // 单机/Host/客户端：先进入闪屏画面，然后自动切换到主场景
                g_mgr->push(new SplashScene(ctx));
        }

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void*, SDL_Event* e)
{
        if (should_close(e)) return SDL_APP_SUCCESS;

        // 独立服务端：只响应退出（不处理菜单/场景切换按键），但允许 ESC 返回主场景，
        // 且仍处理主场景的模式按钮事件（ESC 返回后 mode 仍为 SERVER，需能再切换）
        // 同时需处理 loading_scene 推送的 TRANSITION_EVENT，否则从联机菜单选择"服务端"后卡在加载画面
        if (ctx->mode == MODE_SERVER) {
                g_mgr->HandleEvent(e);
                if (e->type == SDL_EVENT_KEY_DOWN && e->key.scancode == SDL_SCANCODE_ESCAPE) {
                        log_info("[server] ESC pressed, back to main");
                        g_mgr->change(new MainScene(ctx));
                }
                else if (is_user_event(e, TRANSITION_EVENT)) {
                        g_mgr->processPending();
                }
                else if (is_user_event(e, MENU_EVENT)) {
                        switch (get_event_code(e)) {
                        case MENU_EVENT_START:    enter_character_select(MODE_SINGLE); break;
                        case MENU_EVENT_ONLINE:   g_mgr->change(new OnlineScene(ctx)); break;
                        case MENU_EVENT_SETTINGS: g_mgr->change(new SettingsScene(ctx)); break;
                        case MENU_EVENT_EXIT:     push_quit_event(); break;
                        }
                }
                else if (is_user_event(e, ONLINE_EVENT)) {
                        switch (get_event_code(e)) {
                        case ONLINE_EVENT_HOST:   enter_character_select(MODE_HOST); break;
                        case ONLINE_EVENT_CLIENT: enter_character_select(MODE_CLIENT); break;
                        case ONLINE_EVENT_SERVER: enter_game_scene(MODE_SERVER); break;
                        case ONLINE_EVENT_BACK:   g_mgr->change(new MainScene(ctx)); break;
                        }
                }
                else if (is_user_event(e, SETTINGS_EVENT)) {
                        if (get_event_code(e) == SETTINGS_EVENT_BACK)
                                g_mgr->change(new MainScene(ctx));
                }
                else if (is_user_event(e, VICTORY_EVENT)) {
                        handle_victory_event();
                }
                handle_mode_event(e);
                return SDL_APP_CONTINUE;
        }

        g_mgr->HandleEvent(e);

        if (e->type == SDL_EVENT_KEY_DOWN) {
                switch (e->key.scancode) {
                case SDL_SCANCODE_SPACE:
                case SDL_SCANCODE_RETURN:
                        { const char* name = g_mgr->currentName();
                        if (name && strcmp(name, "Main") == 0) {
                                enter_game_scene(ctx->mode);
                        } }
                        break;
                case SDL_SCANCODE_ESCAPE:
                        g_mgr->change(new MainScene(ctx));
                        break;
                case SDL_SCANCODE_R:
                        // 重建场景管理器：先同步清理当前场景资源并删除对象，再推入主场景
                        g_mgr->reset();
                        g_mgr->push(new MainScene(ctx));
                        break;
                }
        }
        else if (is_user_event(e, MENU_EVENT)) {
                switch (get_event_code(e)) {
                case MENU_EVENT_START:
                        enter_character_select(MODE_SINGLE);
                        break;
                case MENU_EVENT_ONLINE:
                        g_mgr->change(new OnlineScene(ctx));
                        break;
                case MENU_EVENT_SETTINGS:
                        g_mgr->change(new SettingsScene(ctx));
                        break;
                case MENU_EVENT_EXIT:
                        push_quit_event();
                        break;
                }
        }
        else if (is_user_event(e, SETTINGS_EVENT)) {
                if (get_event_code(e) == SETTINGS_EVENT_BACK) {
                        g_mgr->change(new MainScene(ctx));
                }
        }
        else if (is_user_event(e, ONLINE_EVENT)) {
                switch (get_event_code(e)) {
                case ONLINE_EVENT_HOST:   enter_character_select(MODE_HOST); break;
                case ONLINE_EVENT_CLIENT: enter_character_select(MODE_CLIENT); break;
                case ONLINE_EVENT_SERVER: enter_game_scene(MODE_SERVER); break;
                case ONLINE_EVENT_BACK:
                        g_mgr->change(new MainScene(ctx));
                        break;
                }
        }
        else if (is_user_event(e, MODE_EVENT)) {
                handle_mode_event(e);
        }
        else if (is_user_event(e, TRANSITION_EVENT)) {
                g_mgr->processPending();
        }
        else if (is_user_event(e, VICTORY_EVENT)) {
                handle_victory_event();
        }

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void*)
{
        game_timer_update(&ctx->game_timer);
        float dt = game_timer_get_delta_time(&ctx->game_timer);
        g_mgr->Update(dt);
        r2d_set_color(ctx->r2d, 255, 255, 255, 255);
        r2d_begin(ctx->r2d);
        g_mgr->Render(ctx->r2d);
        draw_fps_pass(ctx->r2d, AssetManager::GetInstance()->GetSimheiFont24(), dt);
        r2d_end(ctx->r2d);
        return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void*, SDL_AppResult)
{
        delete g_mgr;  // SceneManager 析构：删除当前/过渡场景对象本体（各场景 Destroy 已先清理资源）
        g_mgr = nullptr;
        AssetManager::Destroy();   // 释放字体/纹理缓存（须在 r2d_destroy 之前）
        r2d_destroy(ctx->r2d);
        destroy_window(ctx->window);
        delete ctx;
        sys_release_netenv();
}
