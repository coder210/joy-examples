//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <string.h>
//#include <stdlib.h>
//#include <math.h>
//#include <joy/jconfig.h>
//#include <joy/jscene.h>
//#include <joy/jcore.h>
//
//static SDL_Window*   g_win = NULL;
//static SDL_Renderer* g_ren = NULL;
//static scene_manager_p g_mgr = NULL;
//static scene_node_p g_player = NULL;
//static game_timer_t g_timer;  // 帧率控制器
//static const int W = 800, H = 600;
//
//static void menu_box_render(scene_node_p n, const void* arg) {
//    float x, y;
//    scene_node_get_world_position(n, &x, &y);
//    int z = scene_node_get_zorder(n);
//    SDL_Color c;
//    if (z >= 30)      c = { 80, 160, 255, 255 };
//    else if (z >= 20) c = { 80, 255, 120, 255 };
//    else              c = { 255, 80, 80, 255 };
//    SDL_FRect r = { x - 30, y - 30, 60, 60 };
//    SDL_SetRenderDrawColor(g_ren, c.r, c.g, c.b, c.a);
//    SDL_RenderFillRect(g_ren, &r);
//}
//
//static void menu_box_update(scene_node_p n, float dt) {
//    (void)dt;
//    float bx = 0; float by = 0;
//    // 从 userdata 读取相位偏移，做浮动动画
//    float phase = 0.0f;
//    float* p = (float*)scene_node_get_userdata(n);
//    if (p) phase = *p;
//    static float t = 0.0f;
//    t += 0.016f;
//    // base 坐标存在 scale_x/scale_y 中（偷懒用法）
//    scene_node_get_scale(n, &bx, &by);
//    scene_node_set_position(n, bx + sinf(t * 1.5f + phase) * 30.0f,
//                         by + cosf(t * 1.2f + phase) * 20.0f);
//}
//
//static void menu_box_destroy(scene_node_p n) {
//    float* p = (float*)scene_node_get_userdata(n);
//    free(p);
//}
//
//static void on_menu_load(scene_p s) {
//    (void)s;
//    SDL_Log("[Menu] loaded");
//    float bases[3][2] = { {200,150}, {500,200}, {350,400} };
//    int zs[3] = { 10, 20, 30 };
//    float phases[3] = { 0.0f, 1.5f, 3.0f };
//    for (int i = 0; i < 3; i++) {
//        scene_node_p n = scene_node_create();
//        scene_node_set_position(n, bases[i][0], bases[i][1]);
//        scene_node_set_zorder(n, zs[i]);
//        scene_node_set_scale(n, bases[i][0], bases[i][1]); // 偷存 base
//        scene_node_set_update_callback(n, menu_box_update);
//        scene_node_set_render_callback(n, menu_box_render);
//        scene_node_set_destroy_callback(n, menu_box_destroy);
//        float* ph = (float*)malloc(sizeof(float));
//        *ph = phases[i];
//        scene_node_set_userdata(n, ph);
//        scene_add_root_node(s, n);
//    }
//}
//
//// ============================================================
//// Game 场景
//// ============================================================
//typedef struct { int up, down, left, right; } pdir_t;
//
//static void player_part_render(scene_node_p n, const void* arg) {
//    float x, y;
//    scene_node_get_world_position(n, &x, &y);
//    int z = scene_node_get_zorder(n);
//    SDL_Color c;
//    if (z == 110)       c = { 255, 255, 255, 255 };
//    else if (z == 100)  c = { 200, 200, 210, 255 };
//    else                c = { 120, 120, 140, 255 };
//    SDL_SetRenderDrawColor(g_ren, c.r, c.g, c.b, c.a);
//    int size = (z == 110 || z == 90) ? 16 : 28;
//    SDL_FRect r;
//    r.x = x - size / 2.0f;
//    r.y = y - size / 2.0f;
//    r.w = (float)size;
//    r.h = (float)size;
//    SDL_RenderFillRect(g_ren, &r);
//}
//
//static void enemy_render(scene_node_p n, const void* arg) {
//    float x, y;
//    scene_node_get_world_position(n, &x, &y);
//    SDL_SetRenderDrawColor(g_ren, 255, 100, 100, 255);
//    SDL_FRect r = { x-15, y-15, 30, 30 };
//    SDL_RenderFillRect(g_ren, &r);
//}
//
//static void player_update(scene_node_p n, float dt) {
//    pdir_t* d = (pdir_t*)scene_node_get_userdata(n);
//    if (!d) return;
//    float x, y;
//    scene_node_get_position(n, &x, &y);
//    const float sp = 200.0f;
//    if (d->up)    y -= sp * dt;
//    if (d->down)  y += sp * dt;
//    if (d->left)  x -= sp * dt;
//    if (d->right) x += sp * dt;
//    x = fmaxf(20.0f, fminf((float)W-20.0f, x));
//    y = fmaxf(20.0f, fminf((float)H-20.0f, y));
//    scene_node_set_position(n, x, y);
//}
//
//static void player_destroy(scene_node_p n) {
//    free(scene_node_get_userdata(n));
//}
//
//static void on_game_load(scene_p s) {
//    (void)s;
//    SDL_Log("[Game] loaded");
//    g_player = scene_node_create();
//    scene_node_set_position(g_player, 400, 300);
//    scene_node_set_zorder(g_player, 100);
//    scene_node_set_update_callback(g_player, player_update);
//
//    // 头 (z=110, 相对父节点上方偏移)
//    scene_node_p head = scene_node_create();
//    scene_node_set_position(head, 0, -22);
//    scene_node_set_zorder(head, 110);
//    scene_node_set_render_callback(head, player_part_render);
//    scene_node_add_child(g_player, head);
//
//    // 身 (z=100, 居中)
//    scene_node_p body = scene_node_create();
//    scene_node_set_zorder(body, 100);
//    scene_node_set_render_callback(body, player_part_render);
//    scene_node_add_child(g_player, body);
//
//    // 脚 (z=90, 相对父节点下方偏移)
//    scene_node_p foot = scene_node_create();
//    scene_node_set_position(foot, 0, 22);
//    scene_node_set_zorder(foot, 90);
//    scene_node_set_render_callback(foot, player_part_render);
//    scene_node_add_child(g_player, foot);
//
//    pdir_t* d = (pdir_t*)malloc(sizeof(pdir_t));
//    d->up = d->down = d->left = d->right = 0;
//    scene_node_set_userdata(g_player, d);
//    scene_add_root_node(s, g_player);
//
//    scene_node_p e1 = scene_node_create();
//    scene_node_set_position(e1, 100, 100);
//    scene_node_set_render_callback(e1, enemy_render);
//    scene_add_root_node(s, e1);
//
//    scene_node_p e2 = scene_node_create();
//    scene_node_set_position(e2, 0, 50);
//    scene_node_set_render_callback(e2, enemy_render);
//    scene_node_add_child(e1, e2);
//}
//
//// ============================================================
//// 工厂
//// ============================================================
//static scene_p make_menu(void) {
//    scene_p s = scene_create("Menu");
//    scene_set_load_callback(s, on_menu_load);
//    return s;
//}
//static scene_p make_game(void) {
//    scene_p s = scene_create("Game");
//    scene_set_load_callback(s, on_game_load);
//    return s;
//}
//
//SDL_AppResult SDL_AppInit(void** st, int argc, char* argv[]) {
//    SDL_SetAppMetadata("jscene test", "1.0", "c.jscene");
//    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
//    g_win = SDL_CreateWindow("jscene test", W, H, 0);
//    if (!g_win) return SDL_APP_FAILURE;
//    g_ren = SDL_CreateRenderer(g_win, NULL);
//    if (!g_ren) return SDL_APP_FAILURE;
//    g_mgr = scene_manager_create();
//    game_timer_init(&g_timer);
//    game_timer_set_target_fps(&g_timer, 60);  // 锁定 60 FPS
//    scene_manager_push(g_mgr, make_menu());
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* st, SDL_Event* e) {
//    if (e->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//    if (e->type == SDL_EVENT_KEY_DOWN || e->type == SDL_EVENT_KEY_UP) {
//        bool d = (e->type == SDL_EVENT_KEY_DOWN);
//        switch (e->key.scancode) {
//        case SDL_SCANCODE_SPACE:
//            if (d) { scene_manager_replace(g_mgr, make_game()); }
//            break;
//        case SDL_SCANCODE_ESCAPE:
//            if (d) {
//                scene_manager_replace(g_mgr, make_menu());
//                g_player = NULL;
//            }
//            break;
//        case SDL_SCANCODE_R:
//            if (d) {
//                scene_manager_destroy(g_mgr);
//                g_mgr = scene_manager_create();
//                g_player = NULL;
//                scene_manager_push(g_mgr, make_menu());
//            }
//            break;
//        default:
//            if (g_player) {
//                pdir_t* dir = (pdir_t*)scene_node_get_userdata(g_player);
//                if (dir) {
//                    switch (e->key.scancode) {
//                    case SDL_SCANCODE_UP:    dir->up    = d; break;
//                    case SDL_SCANCODE_DOWN:  dir->down  = d; break;
//                    case SDL_SCANCODE_LEFT:  dir->left  = d; break;
//                    case SDL_SCANCODE_RIGHT: dir->right = d; break;
//                    default: break;
//                    }
//                }
//            }
//            break;
//        }
//    }
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* st) {
//    game_timer_update(&g_timer);                    // 更新帧计时，自动限帧
//    float dt = game_timer_get_delta_time(&g_timer); // 真实 delta time
//    SDL_SetRenderDrawColor(g_ren, 30, 30, 40, 255);
//    SDL_RenderClear(g_ren);
//    scene_manager_update(g_mgr, dt);
//    scene_manager_render(g_mgr, g_ren);
//    // HUD
//    scene_p cur = scene_manager_get_current(g_mgr);
//    const char* txt = cur && strcmp(scene_get_name(cur), "Menu") ? "[Game] Arrows: Move  ESC: Back  R: Reset"
//                                                                 : "[Menu] SPACE: Start  R: Reset";
//    SDL_SetRenderDrawColor(g_ren, 100, 100, 100, 255);
//    SDL_RenderDebugText(g_ren, 10, (float)H-20, txt);
//    SDL_RenderPresent(g_ren);
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* st, SDL_AppResult r) {
//    scene_manager_destroy(g_mgr);
//    SDL_DestroyRenderer(g_ren);
//    SDL_DestroyWindow(g_win);
//}
