//#include "../joy/jbox2d.h"
//#include "SDL3/SDL.h"
//#include "SDL3/SDL_main.h"
//#include <cstdio>
//#include <cstdlib>
//#include <ctime>
//
//// 缺少的 fp_* 宏定义
//#define fp_negate(x)    (-(x))
//#define fp_equal(a,b)   ((a)==(b))
//#define fp_less(a,b)    ((a)<(b))
//#define fp_greater(a,b) ((a)>(b))
//#define fp_lequal(a,b)  ((a)<=(b))
//#define fp_gequal(a,b)  ((a)>=(b))
//
//// C++ 不允许复合字面量 (vec2f_t){x,y}
//static inline vec2f_t V2(fp_t x, fp_t y) { vec2f_t v; v.x = x; v.y = y; return v; }
//
//// ======================== 渲染 & 窗口状态 ========================
//static SDL_Window*    gWindow   = nullptr;
//static SDL_Renderer*  gRenderer = nullptr;
//static int gWidth = 1280, gHeight = 720;
//static float gZoom = 10.0f, gPanY = -8;
//
//static box2d_world_t* gWorld = nullptr;
//static int gDemoIndex = 0;
//static fp_t gTimeStep;
//
//static void SetColor(SDL_Color c) { SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a); }
//static const SDL_Color C_WHITE = {200,200,220,255};
//static const SDL_Color C_GRAY  = {160,170,200,255};
//static const SDL_Color C_RED   = {255, 50, 50, 255};
//
//// ======================== 坐标变换 ========================
//static float WS(float x) {
//    float aspect = (float)gWidth / (float)gHeight;
//    float scale = (gWidth >= gHeight) ? gZoom * aspect : gZoom;
//    return (x / scale + 1.0f) * gWidth * 0.5f;
//}
//static float WY(float y) {
//    float aspect = (float)gWidth / (float)gHeight;
//    float scale = (gWidth >= gHeight) ? gZoom : gZoom / aspect;
//    return (1.0f - (y + gPanY) / scale) * gHeight * 0.5f;
//}
//
//// ======================== 绘制回调 ========================
//static void DrawBody(box2d_body_t* body) {
//    mat22f_t R = mat22f_rotate(body->rotation);
//    vec2f_t x = body->position;
//    vec2f_t h = vec2f_scale(body->bounds, fp_half());
//
//    vec2f_t h1 = mat22f_mul_vec2f(R, V2(fp_negate(h.x), fp_negate(h.y)));
//    vec2f_t h2 = mat22f_mul_vec2f(R, V2(h.x, fp_negate(h.y)));
//    vec2f_t h3 = mat22f_mul_vec2f(R, V2(h.x, h.y));
//    vec2f_t h4 = mat22f_mul_vec2f(R, V2(fp_negate(h.x), h.y));
//
//    vec2f_t v1 = vec2f_add(x, h1), v2 = vec2f_add(x, h2);
//    vec2f_t v3 = vec2f_add(x, h3), v4 = vec2f_add(x, h4);
//
//    float fx[5] = { fp_to_float(v1.x), fp_to_float(v2.x),
//                    fp_to_float(v3.x), fp_to_float(v4.x), fp_to_float(v1.x) };
//    float fy[5] = { fp_to_float(v1.y), fp_to_float(v2.y),
//                    fp_to_float(v3.y), fp_to_float(v4.y), fp_to_float(v1.y) };
//
//    /*
//    SDL_Log("body at world (%f, %f) -> screen (%f, %f)",
//            fp_to_float(body->position.x), fp_to_float(body->position.y),
//            WS(fp_to_float(body->position.x)), WY(fp_to_float(body->position.y)));
//            */
//
//
//    SetColor(C_WHITE);
//    for (int i = 0; i < 4; i++)
//        SDL_RenderLine(gRenderer, WS(fx[i]), WY(fy[i]), WS(fx[i+1]), WY(fy[i+1]));
//}
//
//static void DrawJoint(box2d_joint_t* joint, box2d_body_t* b1, box2d_body_t* b2) {
//    mat22f_t R1 = mat22f_rotate(b1->rotation);
//    mat22f_t R2 = mat22f_rotate(b2->rotation);
//    vec2f_t p1 = vec2f_add(b1->position, mat22f_mul_vec2f(R1, joint->local_anchor1));
//    vec2f_t p2 = vec2f_add(b2->position, mat22f_mul_vec2f(R2, joint->local_anchor2));
//
//    SetColor(C_GRAY);
//    SDL_RenderLine(gRenderer, WS(fp_to_float(p1.x)), WY(fp_to_float(p1.y)),
//                                WS(fp_to_float(p2.x)), WY(fp_to_float(p2.y)));
//}
//
//static void DrawArbiter(box2d_arbiter_t* arbiter) {
//    for (int i = 0; i < arbiter->num_manifolds; ++i) {
//        vec2f_t p = arbiter->manifolds[i].position;
//        SDL_RenderPoint(gRenderer, WS(fp_to_float(p.x)), WY(fp_to_float(p.y)));
//    }
//}
//
//// ======================== Demo 场景 ========================
//static fp_t Random(fp_t lo, fp_t hi) {
//    int r = rand();
//    r /= 0x7ffff;
//    return fp_add(lo, fp_mul(fp_sub(hi, lo), fp_from_float((float)r)));
//}
//
//static void LaunchBomb() {
//    box2d_body_t* bomb = box2d_create_body(gWorld, fp_from_float(50), fp_one(), fp_one());
//    bomb->position  = V2(Random(fp_from_float(-15), fp_from_float(15)), fp_from_float(15));
//    bomb->rotation  = Random(fp_from_float(-1.5f), fp_from_float(1.5f));
//    bomb->velocity  = vec2f_scale(bomb->position, fp_from_float(-1.5f));
//    bomb->angular_velocity = Random(fp_from_float(-20), fp_from_float(20));
//}
//
//static void Demo1() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    box2d_body_t* b2 = box2d_create_body(gWorld, fp_from_float(200), fp_one(), fp_one());
//    b2->position = V2(fp_zero(), fp_from_float(4));
//}
//
//static void Demo2() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    b1->friction = fp_from_float(0.2f);
//    box2d_body_t* b2 = box2d_create_body(gWorld, fp_from_float(100), fp_from_float(1), fp_from_float(1));
//    b2->friction = fp_from_float(0.2f);
//    b2->position = V2(fp_from_float(9), fp_from_float(11));
//    b2->rotation = fp_zero();
//    box2d_create_joint(gWorld, b1->id, b2->id, V2(fp_zero(), fp_from_float(11)));
//}
//
//static void Demo3() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//
//    auto addPlank = [&](fp_t x, fp_t y, fp_t r, fp_t w, fp_t h) {
//        box2d_body_t* b = box2d_create_body(gWorld, fp_max_value(), w, h);
//        b->position = V2(x, y); b->rotation = r; return b;
//    };
//    addPlank(fp_from_float(-2),     fp_from_float(11),    fp_from_float(-0.25f), fp_from_float(13), fp_from_float(0.25f));
//    addPlank(fp_from_float(5.25f),  fp_from_float(9.5f),  fp_zero(),             fp_from_float(0.25f), fp_one());
//    addPlank(fp_from_float(2),      fp_from_float(7),     fp_from_float(0.25f),  fp_from_float(13), fp_from_float(0.25f));
//    addPlank(fp_from_float(-5.25f), fp_from_float(5.5f),  fp_zero(),             fp_from_float(0.25f), fp_one());
//    addPlank(fp_from_float(-2),     fp_from_float(3),     fp_from_float(-0.25f), fp_from_float(13), fp_from_float(0.25f));
//
//    float fs[] = {0.75f,0.5f,0.35f,0.1f,0.0f};
//    for (int i = 0; i < 5; ++i) {
//        box2d_body_t* b = box2d_create_body(gWorld, fp_from_float(25), fp_from_float(0.5f), fp_from_float(0.5f));
//        b->friction = fp_from_float(fs[i]);
//        b->position = V2(fp_add(fp_from_float(-7.5f), fp_from_float(2*i)), fp_from_float(14));
//    }
//}
//
//static void Demo4() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->friction = fp_from_float(0.2f);
//    b1->position = V2(fp_from_float(0), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    for (int i = 0; i < 10; ++i) {
//        box2d_body_t* b = box2d_create_body(gWorld, fp_one(), fp_one(), fp_one());
//        b->friction = fp_from_float(0.2f);
//        b->position = V2(Random(fp_from_float(-0.1f), fp_from_float(0.1f)),
//                          fp_add(fp_from_float(0.51f), fp_mul(fp_from_float(1.05f), fp_from_float(i))));
//    }
//}
//
//static void Demo5() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->friction = fp_from_float(0.2f);
//    b1->position = V2(fp_from_float(0), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    vec2f_t x = V2(fp_from_float(-6), fp_from_float(0.75f));
//    for (int i = 0; i < 12; ++i) {
//        vec2f_t y = x;
//        for (int j = i; j < 12; ++j) {
//            box2d_body_t* b = box2d_create_body(gWorld, fp_from_float(10), fp_one(), fp_one());
//            b->friction = fp_from_float(0.2f);
//            b->position = y;
//            y = vec2f_add(y, V2(fp_from_float(1.125f), fp_zero()));
//        }
//        x = vec2f_add(x, V2(fp_from_float(0.5625f), fp_from_float(2)));
//    }
//}
//
//static void Demo6() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    box2d_body_t* b2 = box2d_create_body(gWorld, fp_from_float(100), fp_from_float(12), fp_from_float(0.25f));
//    b2->position = V2(fp_zero(), fp_one());
//    box2d_body_t* b3 = box2d_create_body(gWorld, fp_from_float(25), fp_from_float(0.5f), fp_from_float(0.5f));
//    b3->position = V2(fp_from_float(-5), fp_from_float(2));
//    box2d_body_t* b4 = box2d_create_body(gWorld, fp_from_float(25), fp_from_float(0.5f), fp_from_float(0.5f));
//    b4->position = V2(fp_from_float(-5.5f), fp_from_float(2));
//    box2d_body_t* b5 = box2d_create_body(gWorld, fp_from_float(100), fp_one(), fp_one());
//    b5->position = V2(fp_from_float(5.5f), fp_from_float(15));
//    box2d_create_joint(gWorld, b1->id, b2->id, V2(fp_zero(), fp_one()));
//}
//
//static void Demo7() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    b1->friction = fp_from_float(0.2f);
//
//    box2d_body_t* bodies[16];
//    bodies[0] = b1;
//    int np = 15;
//    fp_t mass = fp_from_float(50);
//    for (int i = 0; i < np; ++i) {
//        box2d_body_t* b = box2d_create_body(gWorld, mass, fp_one(), fp_from_float(0.25f));
//        b->friction = fp_from_float(0.2f);
//        b->position = V2(fp_add(fp_from_float(-8.5f), fp_mul(fp_from_float(1.25f), fp_from_float(i))), fp_from_float(5));
//        bodies[i + 1] = b;
//    }
//    fp_t omega = fp_mul(fp_from_float(2), fp_mul(fp_from_float(3.141592658f), fp_from_float(2)));
//    fp_t d     = fp_mul(fp_mul(fp_from_float(2), mass), fp_mul(fp_from_float(0.7f), omega));
//    fp_t k     = fp_mul(mass, fp_pow2(omega));
//    fp_t soft  = fp_div(fp_one(), fp_add(d, fp_mul(gTimeStep, k)));
//    fp_t bias  = fp_div(fp_mul(gTimeStep, k), fp_add(d, fp_mul(gTimeStep, k)));
//
//    for (int i = 0; i < np; ++i) {
//        box2d_joint_t* j = box2d_create_joint(gWorld, bodies[i]->id, bodies[i+1]->id,
//            V2(fp_add(fp_from_float(-9.125f), fp_mul(fp_from_float(1.25f), fp_from_float(i))), fp_from_float(5)));
//        j->softness = soft; j->biasfactor = bias;
//    }
//    box2d_joint_t* j = box2d_create_joint(gWorld, bodies[np]->id, bodies[0]->id,
//        V2(fp_add(fp_from_float(-9.125f), fp_mul(fp_from_float(1.25f), fp_from_float(np))), fp_from_float(5)));
//    j->softness = soft; j->biasfactor = bias;
//}
//
//static void Demo8() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    box2d_body_t* b12 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(12), fp_from_float(0.5f));
//    b12->position = V2(fp_from_float(-1.5f), fp_from_float(10));
//    for (int i = 0; i < 10; ++i) {
//        box2d_body_t* b = box2d_create_body(gWorld, fp_from_float(10), fp_from_float(0.2f), fp_from_float(2));
//        b->position = V2(fp_add(fp_from_float(-6), fp_from_float(i)), fp_from_float(11.125f));
//        b->friction = fp_from_float(0.1f);
//    }
//    box2d_body_t* b13 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(14), fp_from_float(0.5f));
//    b13->position = V2(fp_from_float(1), fp_from_float(6)); b13->rotation = fp_from_float(0.3f);
//    box2d_body_t* b2 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(0.5f), fp_from_float(3));
//    b2->position = V2(fp_from_float(-7), fp_from_float(4));
//    box2d_body_t* b3 = box2d_create_body(gWorld, fp_from_float(20), fp_from_float(12), fp_from_float(0.25f));
//    b3->position = V2(fp_from_float(-0.9f), fp_one());
//    box2d_create_joint(gWorld, b1->id, b3->id, V2(fp_from_float(-2), fp_one()));
//    box2d_body_t* b4 = box2d_create_body(gWorld, fp_from_float(10), fp_from_float(0.5f), fp_from_float(0.5f));
//    b4->position = V2(fp_from_float(-10), fp_from_float(15));
//    box2d_create_joint(gWorld, b2->id, b4->id, V2(fp_from_float(-7), fp_from_float(15)));
//    box2d_body_t* b5 = box2d_create_body(gWorld, fp_from_float(20), fp_from_float(2), fp_from_float(2));
//    b5->position = V2(fp_from_float(6), fp_from_float(2.5f)); b5->friction = fp_from_float(0.1f);
//    box2d_create_joint(gWorld, b1->id, b5->id, V2(fp_from_float(6), fp_from_float(2.6f)));
//    box2d_body_t* b6 = box2d_create_body(gWorld, fp_from_float(10), fp_from_float(2), fp_from_float(0.2f));
//    b6->position = V2(fp_from_float(6), fp_from_float(3.6f));
//    box2d_create_joint(gWorld, b5->id, b6->id, V2(fp_from_float(7), fp_from_float(3.5f)));
//}
//
//static void Demo9() {
//    box2d_body_t* b1 = box2d_create_body(gWorld, fp_max_value(), fp_from_float(100), fp_from_float(20));
//    b1->position = V2(fp_zero(), fp_mul(fp_from_float(-0.5f), b1->bounds.y));
//    b1->friction = fp_from_float(0.2f);
//
//    fp_t mass = fp_from_float(10);
//    fp_t omega = fp_mul(fp_from_float(2), fp_mul(fp_from_float(3.141592658f), fp_from_float(4)));
//    fp_t d     = fp_mul(fp_mul(fp_from_float(2), mass), fp_mul(fp_from_float(0.7f), omega));
//    fp_t k     = fp_mul(mass, fp_pow2(omega));
//    fp_t soft  = fp_div(fp_one(), fp_add(d, fp_mul(gTimeStep, k)));
//    fp_t bias  = fp_div(fp_mul(gTimeStep, k), fp_add(d, fp_mul(gTimeStep, k)));
//
//    for (int i = 0; i < 15; ++i) {
//        box2d_body_t* b = box2d_create_body(gWorld, mass, fp_from_float(0.75f), fp_from_float(0.25f));
//        b->friction = fp_from_float(0.2f);
//        b->position = V2(fp_add(fp_from_float(0.5f), fp_from_float(i)), fp_from_float(12));
//        box2d_joint_t* j = box2d_create_joint(gWorld, b1->id, b->id, V2(fp_from_float(i), fp_from_float(12)));
//        j->softness = soft; j->biasfactor = bias;
//        b1 = b;
//    }
//}
//
//static void (*gDemos[])() = { Demo1, Demo2, Demo3, Demo4, Demo5, Demo6, Demo7, Demo8, Demo9 };
//static const char* gDemoNames[] = {
//    "Demo 1: A Single Box",       "Demo 2: Simple Pendulum",
//    "Demo 3: Varying Friction",   "Demo 4: Randomized Stacking",
//    "Demo 5: Pyramid Stacking",   "Demo 6: A Teeter",
//    "Demo 7: A Suspension Bridge","Demo 8: Dominos",
//    "Demo 9: Multi-pendulum"
//};
//
//static void InitDemo(int idx) {
//    box2d_clear_world(gWorld);
//    gDemoIndex = idx;
//    gDemos[idx]();
//}
//
//// ======================== 输入处理 ========================
//static void HandleKey(SDL_Keycode key) {
//    if (key >= SDLK_1 && key <= SDLK_9) InitDemo((int)(key - SDLK_1));
//    else if (key == SDLK_SPACE) LaunchBomb();
//}
//
//// ======================== 主入口 ========================
//int main(int, char**) {
//    srand(0);
//
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed");
//        return 1;
//    }
//    gWindow = SDL_CreateWindow("Box2D Demo", gWidth, gHeight, 0);
//    if (!gWindow) return 1;
//    gRenderer = SDL_CreateRenderer(gWindow, NULL);
//    if (!gRenderer) return 1;
//
//    gWorld = box2d_create_world(V2(fp_zero(), fp_from_float(-10)), 10);
//    gTimeStep = fp_div(fp_one(), fp_from_float(60));
//
//    InitDemo(7);
//
//    bool running = true;
//    while (running) {
//        SDL_Event e;
//        while (SDL_PollEvent(&e)) {
//            if (e.type == SDL_EVENT_QUIT) running = false;
//            else if (e.type == SDL_EVENT_KEY_DOWN) {
//                if (e.key.key == SDLK_ESCAPE) running = false;
//                HandleKey(e.key.key);
//            }
//        }
//
//        box2d_step(gWorld, gTimeStep);
//
//        SDL_SetRenderDrawColor(gRenderer, 10, 10, 18, 255);
//        SDL_RenderClear(gRenderer);
//
//        box2d_foreach_bodies(gWorld, DrawBody);
//        box2d_foreach_joints(gWorld, DrawJoint);
//
//        SetColor(C_RED);
//        box2d_foreach_arbiters(gWorld, DrawArbiter);
//
//        SDL_RenderPresent(gRenderer);
//        SDL_Delay(16);
//    }
//
//    box2d_destroy_world(gWorld);
//    SDL_DestroyRenderer(gRenderer);
//    SDL_DestroyWindow(gWindow);
//    SDL_Quit();
//    return 0;
//}
