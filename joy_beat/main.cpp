#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <joy/joy.h>

/* ===== 脚本索引（enum 自动编号，新增插入在 IDX_COUNT 前即可） ===== */
enum {
        IDX_00_HELLO_LUA,
        IDX_00_HELLO_JS,
        IDX_01_CRYPT_LUA,
        IDX_01_CRYPT_JS,
        IDX_02_TEST_ECS_LUA,
        IDX_02_TEST_ECS_JS,
        IDX_03_KCP_CLIENT_LUA,
        IDX_03_KCP_CLIENT_JS,
        IDX_04_KCP_SERVER_LUA,
        IDX_04_KCP_SERVER_JS,
        IDX_05_COLLISION_CIRCLES_LUA,
        IDX_05_COLLISION_CIRCLES_JS,
        IDX_06_COLLISION_POLYGONS_LUA,
        IDX_06_COLLISION_POLYGONS_JS,
        IDX_07_COLLISION_RAY_QUAD_LUA,
        IDX_07_COLLISION_RAY_QUAD_JS,
        IDX_08_COLLISION_RAY_RECTANGLE_LUA,
        IDX_08_COLLISION_RAY_RECTANGLE_JS,
        IDX_09_COLLISION_RECTANGLES_LUA,
        IDX_09_COLLISION_RECTANGLES_JS,
        IDX_10_GRAPHICS_LUA,
        IDX_10_GRAPHICS_JS,
        IDX_11_AUDIO_LUA,
        IDX_11_AUDIO_JS,
        IDX_12_UI_LUA,
        IDX_12_UI_JS,
        IDX_13_GPU_LUA,
        IDX_13_GPU_JS,
        IDX_14_FPS_LUA,
        IDX_14_FPS_JS,
        IDX_15_ANIMATOR_LUA,
        IDX_15_ANIMATOR_JS,
        IDX_JOY_LUA,
        IDX_MAIN_LUA,
        IDX_MAIN_JS,
        IDX_WORKER_LUA,
        IDX_WORKER_JS,
        IDX_16_BEHAVIOR_TREE_LUA,
        IDX_16_BEHAVIOR_TREE_JS,
        IDX_17_SCENE_LUA,
        IDX_17_SCENE_JS,
        IDX_18_MSGPACK_LUA,
        IDX_18_MSGPACK_JS,
        IDX_19_ADVENTURE_LUA,
        IDX_19_ADVENTURE_CLIENT_LUA,
        IDX_20_UI_PROCESS_EVENT_LUA,
        IDX_20_UI_PROCESS_EVENT_JS,
        IDX_COUNT
};

#define DEFAULT_SCRIPT_IDX IDX_20_UI_PROCESS_EVENT_LUA
static const char* script_list[IDX_COUNT] = {
        "./joy_beat/scripts/00-hello.lua",
        "./joy_beat/scripts/00-hello.js",
        "./joy_beat/scripts/01-crypt.lua",
        "./joy_beat/scripts/01-crypt.js",
        "./joy_beat/scripts/02-test-ecs.lua",
        "./joy_beat/scripts/02-test-ecs.js",
        "./joy_beat/scripts/03-kcp-client.lua",
        "./joy_beat/scripts/03-kcp-client.js",
        "./joy_beat/scripts/04-kcp-server.lua",
        "./joy_beat/scripts/04-kcp-server.js",
        "./joy_beat/scripts/05-collision-circles.lua",
        "./joy_beat/scripts/05-collision-circles.js",
        "./joy_beat/scripts/06-collision-polygons.lua",
        "./joy_beat/scripts/06-collision-polygons.js",
        "./joy_beat/scripts/07-collision-ray-quad.lua",
        "./joy_beat/scripts/07-collision-ray-quad.js",
        "./joy_beat/scripts/08-collision-ray-rectangle.lua",
        "./joy_beat/scripts/08-collision-ray-rectangle.js",
        "./joy_beat/scripts/09-collision-rectangles.lua",
        "./joy_beat/scripts/09-collision-rectangles.js",
        "./joy_beat/scripts/10-graphics.lua",
        "./joy_beat/scripts/10-graphics.js",
        "./joy_beat/scripts/11-audio.lua",
        "./joy_beat/scripts/11-audio.js",
        "./joy_beat/scripts/12-ui.lua",
        "./joy_beat/scripts/12-ui.js",
        "./joy_beat/scripts/13-gpu.lua",
        "./joy_beat/scripts/13-gpu.js",
        "./joy_beat/scripts/14-fps.lua",
        "./joy_beat/scripts/14-fps.js",
        "./joy_beat/scripts/15-animator.lua",
        "./joy_beat/scripts/15-animator.js",
        "./joy_beat/scripts/joy.lua",
        "./joy_beat/scripts/main.lua",
        "./joy_beat/scripts/main.js",
        "./joy_beat/scripts/worker.lua",
        "./joy_beat/scripts/worker.js",
        "./joy_beat/scripts/16-behavior-tree.lua",
        "./joy_beat/scripts/16-behavior-tree.js",
        "./joy_beat/scripts/17-scene.lua",
        "./joy_beat/scripts/17-scene.js",
        "./joy_beat/scripts/18-msgpack.lua",
        "./joy_beat/scripts/18-msgpack.js",
        "./joy_beat/scripts/19-adventure.lua",
        "./joy_beat/scripts/19-adventure-client.lua",
        "./joy_beat/scripts/20-ui-process-event.lua",
        "./joy_beat/scripts/20-ui-process-event.js",
};
#define SCRIPT_COUNT IDX_COUNT

typedef struct {
        char* file_path;
        bool is_lua;
        union {
                luax_p lua;
                jsx_p js;
        };
        game_timer_t timer;
} app_state_t;


static void* create_script(app_state_t* s)
{
        if (s->is_lua) { s->lua = luax_create(); return s->lua; }
        s->js = jsx_create();
        return s->js;
}

static bool reload(app_state_t* s, const char* file)
{
        if (s->is_lua) return luax_reload(s->lua, file);
        return jsx_reload(s->js, file);
}

static bool call_joy(app_state_t* s, const char* func, int argc, float dt)
{
        if (s->is_lua) return luax_call_joy(s->lua, func, argc, dt);
        return jsx_call_joy(s->js, func, argc, dt);
}

static void set_watch(app_state_t* s, const char* file)
{
        if (s->is_lua) luax_set_watch(s->lua, file);
        else jsx_set_watch(s->js, file);
}

static bool poll_reload(app_state_t* s)
{
        if (s->is_lua) return luax_poll_reload(s->lua);
        return jsx_poll_reload(s->js);
}

static void dispatch_event(app_state_t* s, const SDL_Event* event)
{
        if (s->is_lua) luax_dispatch_event(s->lua, event);
        else jsx_dispatch_event(s->js, event);
}

static void set_args(app_state_t* s, int argc, char* argv[])
{
        /* 跳过 argv[0](exe) 和 argv[1](脚本路径), 只传额外参数 */
        int script_argc = argc > 2 ? argc - 2 : 0;
        char** script_argv = argc > 2 ? argv + 2 : NULL;
        if (s->is_lua) luax_set_args(s->lua, script_argc, script_argv);
        else jsx_set_args(s->js, script_argc, script_argv);
}

static void release_script(app_state_t* s)
{
        if (s->is_lua) luax_release(s->lua);
        else jsx_release(s->js);
}


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
        init_env();
        sys_init_netenv();

        // 获取电源状态
        //int seconds, percent;
        //SDL_PowerState ps = SDL_GetPowerInfo(&seconds, &percent);
        //if (ps == SDL_POWERSTATE_ON_BATTERY);


        app_state_t* s = (app_state_t*)SDL_calloc(1, sizeof(app_state_t));
        *appstate = s;

        if (argc > 1) {
                s->file_path = SDL_strdup(argv[1]);
        }
        else if (DEFAULT_SCRIPT_IDX >= 0 && DEFAULT_SCRIPT_IDX < SCRIPT_COUNT) {
                s->file_path = SDL_strdup(script_list[DEFAULT_SCRIPT_IDX]);
                log_info("default script [%d]: %s", DEFAULT_SCRIPT_IDX, s->file_path);
        }
        else {
                s->file_path = SDL_strdup(script_list[0]);
        }

        /* 根据后缀名选择脚本引擎 */
        const char* ext = SDL_strrchr(s->file_path, '.');
        if (ext && SDL_strcasecmp(ext, ".js") == 0)
                s->is_lua = false;
        else
                s->is_lua = true;   /* 默认 .lua */

        log_info("script engine: %s", s->is_lua ? "luax" : "jsx");

        if (!create_script(s)) {
                log_info("script engine create failed");
                return SDL_APP_FAILURE;
        }

        /* 首次加载 */
        log_info("--- loading: %s ---", s->file_path);
        if (!reload(s, s->file_path)) {
                log_info("ERROR: failed to load script: %s", s->file_path);
                return SDL_APP_FAILURE;
        }
        set_args(s, argc, argv);
        call_joy(s, "load", 0, 0);

        /* 启用文件监控 + 帧率限制 60 FPS */
        set_watch(s, s->file_path);
        game_timer_init(&s->timer);
        game_timer_set_target_fps(&s->timer, 60);

        log_info("joy_beat - watching: %s", s->file_path);
        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
        app_state_t* s = (app_state_t*)appstate;

        game_timer_update(&s->timer);
        float dt = game_timer_get_delta_time(&s->timer);

        poll_reload(s);
        call_joy(s, "update", 1, dt);
        call_joy(s, "draw", 0, 0);

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
        app_state_t* s = (app_state_t*)appstate;
        if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
        dispatch_event(s, event);
        return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
        app_state_t* s = (app_state_t*)appstate;
        if (s) {
                if (s->file_path) {
                        call_joy(s, "quit", 0, 0);
                        release_script(s);
                }
                SDL_free(s->file_path);
                SDL_free(s);
        }
        (void)result;
        sys_release_netenv();
}
