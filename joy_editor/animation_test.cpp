//// animation_test.cpp — 精灵帧动画测试
//// hlw.png: 128×171, 9列×6行, 每行一个动画
//// 空格切换动画
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <cstdio>
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <joy/janimations.h>
//#include <joy/jcore.h>
//
//static const int W = 800, H = 600;
//static SDL_Window*   gWin = nullptr;
//static SDL_Renderer* gRen = nullptr;
//
//static sprite_animation_p gAnim[8] = {};
//static int gCurrAnim = 0;
//static simple_fps_counter_p gFps = nullptr;
//static Uint64 gFrameStart = 0;
//
//// 精灵图参数
//static const int ROWS = 8;
//static const int FRAMES_PER_ROW = 6;
//static const float DISPLAY_W = 150.0f;
//static const float DISPLAY_H = 150.0f;
//static const float FRAME_DURATION = 0.1f;
//
//static const char* TEX_PATH = "joy2d_editor_textures/hlw2.png";
//static const char* ANIM_NAMES[] = {
//    "Anim 1", "Anim 2", "Anim 3", "Anim 4", "Anim 5", "Anim 6", "Anim 7", "Anim 8"
//};
//
//static void create_animations() {
//    for (int row = 0; row < ROWS; row++) {
//        gAnim[row] = sprite_animation_create(gRen);
//        if (!gAnim[row]) continue;
//        for (int col = 0; col < FRAMES_PER_ROW; col++) {
//            SDL_FRect src = {
//                (float)col * DISPLAY_W,
//                (float)row * DISPLAY_H,
//                DISPLAY_W,
//                DISPLAY_H
//            };
//            sprite_animation_add_clip(gAnim[row], TEX_PATH, FRAME_DURATION, src);
//        }
//        sprite_animation_set_position(gAnim[row], W/2.0f, H/2.0f);
//        sprite_animation_set_scale(gAnim[row], 1, 1);
//    }
//}
//
//static void switch_anim() {
//    gCurrAnim = (gCurrAnim + 1) % ROWS;
//}
//
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
//    (void)argc; (void)argv; *appstate = nullptr;
//    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
//    if (!SDL_CreateWindowAndRenderer("Animation Test", W, H, 0, &gWin, &gRen))
//        return SDL_APP_FAILURE;
//    gFps = simple_fps_create();
//    gFrameStart = SDL_GetTicksNS();
//    create_animations();
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* ev) {
//    (void)appstate;
//    if (ev->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//    if (ev->type == SDL_EVENT_KEY_DOWN) {
//        if (ev->key.scancode == SDL_SCANCODE_ESCAPE) return SDL_APP_SUCCESS;
//        if (ev->key.scancode == SDL_SCANCODE_SPACE) switch_anim();
//    }
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//    float dt = 1.0f / 60.0f;
//    for (int i = 0; i < ROWS; i++)
//        if (gAnim[i]) sprite_animation_update(gAnim[i], dt);
//
//    SDL_SetRenderDrawColor(gRen, 30, 30, 40, 255);
//    SDL_RenderClear(gRen);
//
//    if (gAnim[gCurrAnim])
//        sprite_animation_draw(gAnim[gCurrAnim], NULL);
//
//    char title[128];
//    int fps = gFps ? simple_fps_value(gFps) : 0;
//    SDL_snprintf(title, sizeof(title), "Animation: %s | FPS: %d | SPACE to switch", ANIM_NAMES[gCurrAnim], fps);
//    SDL_SetWindowTitle(gWin, title);
//
//    SDL_RenderPresent(gRen);
//
//    // 限速60fps
//    Uint64 now = SDL_GetTicksNS();
//    Uint64 elapsed = now - gFrameStart;
//    Uint64 target = 16666667ull; // 1/60 秒 ≈ 16.67ms
//    if (elapsed < target) SDL_DelayNS((Uint32)(target - elapsed));
//    gFrameStart = SDL_GetTicksNS();
//    if (gFps) simple_fps_update(gFps);
//
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate; (void)result;
//    for (int i = 0; i < ROWS; i++)
//        if (gAnim[i]) sprite_animation_destroy(gAnim[i]);
//    if (gFps) simple_fps_destory(gFps);
//    SDL_DestroyRenderer(gRen);
//    SDL_DestroyWindow(gWin);
//}
