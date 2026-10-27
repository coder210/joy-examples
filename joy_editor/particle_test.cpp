//// particle_test.cpp — 粒子系统交互式测试
//// 编译: cmake -DBUILD_PARTICLE_TEST=ON ..
//// 展示多种粒子效果: 爆炸/喷泉/火焰/拖尾
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <cstdio>
//#include <cstdlib>
//#include <cmath>
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <joy/jparticle.h>
//
//static const int WINDOW_W = 1024, WINDOW_H = 768;
//static SDL_Window*   gWindow  = nullptr;
//static SDL_Renderer* gRenderer = nullptr;
//
//// 多种粒子发射器
//static jparticle_p gEmitter[4] = {};
//static int gActive = 0;     // 当前选择的发射器
//
//static const char* gNames[] = {
//    "1-Burst (爆炸)",
//    "2-Fountain (喷泉)",
//    "3-Fire (火焰)",
//    "4-Trail (拖尾)"
//};
//
//static void switch_emitter(int idx) {
//    gActive = (idx + 4) % 4;
//}
//
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
//    (void)argc; (void)argv;
//    *appstate = nullptr;
//
//    srand((unsigned int)SDL_GetTicks());
//
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        SDL_Log("SDL_Init: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//    if (!SDL_CreateWindowAndRenderer("Particle Test", WINDOW_W, WINDOW_H, 0,
//                                     &gWindow, &gRenderer)) {
//        SDL_Log("SDL_Window: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    // ── 1. 爆炸(Burst): 红色/橙色 向四面八方扩散 ──
//    {
//        jparticle_cfg_t cfg = jparticle_cfg_default();
//        cfg.x = 300; cfg.y = 400;
//        cfg.direction = 0;   cfg.spread = 360;
//        cfg.speed_min = 80;  cfg.speed_max = 200;
//        cfg.lifetime_min = 0.6f; cfg.lifetime_max = 1.2f;
//        cfg.size_start = 6;  cfg.size_end = 1;
//        cfg.r = 255; cfg.g = 120; cfg.b = 30; cfg.a = 255;
//        cfg.burst = 60;
//        cfg.rate = 0;        // 不持续
//        cfg.fadeout = 0.7f;
//        gEmitter[0] = jparticle_create(cfg);
//    }
//
//    // ── 2. 喷泉: 向上喷射, 重力下落 ──
//    {
//        jparticle_cfg_t cfg = jparticle_cfg_default();
//        cfg.x = 400; cfg.y = 650;
//        cfg.direction = -90; cfg.spread = 20;
//        cfg.speed_min = 150; cfg.speed_max = 250;
//        cfg.lifetime_min = 1.5f; cfg.lifetime_max = 3.0f;
//        cfg.size_start = 5;  cfg.size_end = 2;
//        cfg.r = 100; cfg.g = 200; cfg.b = 255; cfg.a = 200;
//        cfg.burst = 3;
//        cfg.rate = 60;       // 每秒60个
//        cfg.gravity = true;
//        cfg.gravity_y = 80.0f;
//        cfg.fadeout = 0.6f;
//        gEmitter[1] = jparticle_create(cfg);
//    }
//
//    // ── 3. 火焰: 向上飘散, 红→黄→透明 ──
//    {
//        jparticle_cfg_t cfg = jparticle_cfg_default();
//        cfg.x = 600; cfg.y = 500;
//        cfg.direction = -90; cfg.spread = 35;
//        cfg.speed_min = 30;  cfg.speed_max = 80;
//        cfg.lifetime_min = 0.4f; cfg.lifetime_max = 1.0f;
//        cfg.size_start = 12; cfg.size_end = 2;
//        cfg.r = 255; cfg.g = 200; cfg.b = 50; cfg.a = 200;
//        cfg.burst = 2;
//        cfg.rate = 80;
//        cfg.spawn_radius = 5;
//        cfg.fadeout = 0.4f;
//        gEmitter[2] = jparticle_create(cfg);
//    }
//
//    // ── 4. 拖尾: 跟随鼠标连发 ──
//    {
//        jparticle_cfg_t cfg = jparticle_cfg_default();
//        cfg.x = 800; cfg.y = 400;
//        cfg.direction = -90; cfg.spread = 45;
//        cfg.speed_min = 20;  cfg.speed_max = 60;
//        cfg.lifetime_min = 0.3f; cfg.lifetime_max = 0.8f;
//        cfg.size_start = 4;  cfg.size_end = 0;
//        cfg.r = 255; cfg.g = 100; cfg.b = 255; cfg.a = 180;
//        cfg.burst = 1;
//        cfg.rate = 0;        // 鼠标点击时爆发
//        cfg.fadeout = 0.5f;
//        gEmitter[3] = jparticle_create(cfg);
//    }
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    (void)appstate;
//
//    // 按键切换/触发
//    if (event->type == SDL_EVENT_KEY_DOWN) {
//        switch (event->key.scancode) {
//        case SDL_SCANCODE_ESCAPE:
//        case SDL_SCANCODE_Q:
//            return SDL_APP_SUCCESS;
//        case SDL_SCANCODE_1: switch_emitter(0); break;
//        case SDL_SCANCODE_2: switch_emitter(1); break;
//        case SDL_SCANCODE_3: switch_emitter(2); break;
//        case SDL_SCANCODE_4: switch_emitter(3); break;
//        case SDL_SCANCODE_SPACE:
//            // 对当前发射器爆发一次
//            jparticle_emit(gEmitter[gActive]);
//            break;
//        default: break;
//        }
//    }
//
//    // 鼠标: 移动更新位置, 点击爆发
//    if (event->type == SDL_EVENT_MOUSE_MOTION) {
//        // 拖尾发射器跟随鼠标
//        jparticle_set_position(gEmitter[3], event->motion.x, event->motion.y);
//        if (event->motion.state & SDL_BUTTON_LMASK) {
//            jparticle_emit(gEmitter[3]);  // 按住时持续爆发
//        }
//    }
//    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
//        // 爆炸发射器在点击位置爆发
//        if (event->button.button == SDL_BUTTON_LEFT) {
//            jparticle_set_position(gEmitter[0], event->button.x, event->button.y);
//            jparticle_emit(gEmitter[0]);
//        }
//    }
//
//    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    float dt = 1.0f / 60.0f; // 固定步长
//
//    // 更新全部发射器
//    for (int i = 0; i < 4; i++)
//        jparticle_update(gEmitter[i], dt);
//
//    // 渲染
//    SDL_SetRenderDrawColor(gRenderer, 20, 20, 30, 255);
//    SDL_RenderClear(gRenderer);
//
//    for (int i = 0; i < 4; i++)
//        jparticle_render(gEmitter[i], gRenderer);
//
//    // 提示文字(简单用窗口标题)
//    {
//        char title[256];
//        int total = 0;
//        for (int i = 0; i < 4; i++) total += jparticle_count(gEmitter[i]);
//        SDL_snprintf(title, sizeof(title),
//            "Particle Test | %s | particles: %d | "
//            "1-4:switch SPACE:burst LMB:explode",
//            gNames[gActive], total);
//        SDL_SetWindowTitle(gWindow, title);
//    }
//
//    SDL_RenderPresent(gRenderer);
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate;
//    for (int i = 0; i < 4; i++)
//        if (gEmitter[i]) jparticle_destroy(gEmitter[i]);
//    if (gRenderer) SDL_DestroyRenderer(gRenderer);
//    if (gWindow) SDL_DestroyWindow(gWindow);
//}
