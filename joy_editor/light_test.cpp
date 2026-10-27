//// light_test.cpp — 最简测试: 一个光源 + 一个遮挡物
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <cstdio>
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <joy/jlight.h>
//
//static SDL_Window*   gWin = nullptr;
//static SDL_Renderer* gRen = nullptr;
//static jlight_sys_p  gSys = nullptr;
//static int gLightId = -1;
//static bool gDragging = false;
//static float gOffX=0, gOffY=0;
//
//static const int W = 800, H = 600;
//
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
//    (void)argc; (void)argv; *appstate = nullptr;
//    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
//    if (!SDL_CreateWindowAndRenderer("Light Test", W, H, 0, &gWin, &gRen)) return SDL_APP_FAILURE;
//
//    gSys = jlight_sys_create(W, H);
//    jlight_set_ambient(gSys, 15, 15, 20);
//
//    // 一面墙
//    jlight_add_occluder(gSys, 350, 200, 350, 400);
//
//    // 一个光源
//    jlight_t l = {200, 300, 500, 255, 255, 200, 1.0f};
//    gLightId = jlight_add(gSys, l);
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* ev) {
//    (void)appstate;
//    if (ev->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//    if (ev->type == SDL_EVENT_KEY_DOWN && ev->key.scancode == SDL_SCANCODE_ESCAPE) return SDL_APP_SUCCESS;
//
//    float mx = ev->button.x, my = ev->button.y;
//    if (ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev->button.button == SDL_BUTTON_LEFT) {
//        jlight_t* l = &gSys->lights[gLightId];
//        float dx = mx - l->x, dy = my - l->y;
//        if (dx*dx + dy*dy < 400) { gDragging = true; gOffX = dx; gOffY = dy; }
//    }
//    if (ev->type == SDL_EVENT_MOUSE_BUTTON_UP) gDragging = false;
//    if (ev->type == SDL_EVENT_MOUSE_MOTION && gDragging) {
//        gSys->lights[gLightId].x = ev->motion.x - gOffX;
//        gSys->lights[gLightId].y = ev->motion.y - gOffY;
//    }
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    // 深灰背景
//    SDL_SetRenderDrawColor(gRen, 40, 40, 40, 255);
//    SDL_RenderClear(gRen);
//
//    // 浅灰地面
//    SDL_SetRenderDrawColor(gRen, 80, 80, 80, 255);
//    SDL_RenderFillRect(gRen, NULL);
//
//    // 画墙
//    SDL_SetRenderDrawColor(gRen, 200, 80, 80, 255);
//    for (int i = 0; i < gSys->occluder_count; i++) {
//        jlight_occluder_t* o = &gSys->occluders[i];
//        SDL_RenderLine(gRen, o->x1, o->y1, o->x2, o->y2);
//    }
//
//    // 光照
//    jlight_render(gSys, gRen);
//
//    // 光源标记
//    jlight_t* l = &gSys->lights[gLightId];
//    SDL_SetRenderDrawColor(gRen, 255, 255, 100, 255);
//    SDL_FRect dot = {l->x - 5, l->y - 5, 10, 10};
//    SDL_RenderFillRect(gRen, &dot);
//
//    SDL_RenderPresent(gRen);
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate; (void)result;
//    if (gSys) jlight_sys_destroy(gSys);
//    SDL_DestroyRenderer(gRen);
//    SDL_DestroyWindow(gWin);
//}
