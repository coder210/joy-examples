//// main_tex.cpp — 用 SDL_GPU (通过 jr3d 库) 渲染一张图片(纹理贴图四边形)
////
//// 复刻自独立 SDL_GPU 纹理示例: 加载一张 PNG, 创建 GPU 纹理, 用一个带 UV 的
//// quad 把它画到屏幕上。这里复用项目已验证可用的 jr3d 纹理管线
//// (r3d_create_texture + r3d_draw_mesh_tex / r3d_draw_mesh_tex_basis),
//// 避免手写 SPIR-V。
////
//// 编译约定: joy_editor 下同时只能有一个带 SDL_AppInit 入口的文件参与编译,
//// 使用时请把其他 main*.cpp 的 SDL_AppInit 入口注释掉(本文件作为唯一入口)。
//
//#define SDL_MAIN_USE_CALLBACKS
//#include "SDL3/SDL.h"
//#include "SDL3/SDL_main.h"
//#include <joy/joy.h>
//#include <iostream>
//#include <vector>
//#include <cstring>
//#include <cstdio>
//
//// -------------------- 全局资源 --------------------
//static jwindow_p gWin = nullptr;
//static SDL_Window* gSDLWin = nullptr;   // 用于 SDL_StartTextInput (参考 jr2d inputbox_focus 从 renderer 直接取窗口)
//static jr3d_p    gR3d = nullptr;
//
//static render_mesh_p gQuad = nullptr;     // 单位 quad (pos+uv)
//static void*          gTex  = nullptr;    // 图片纹理
//
//// 相机参数
//static vec3_t gCamPos   = { 0.0f, 1.0f, 4.0f };
//static float  gCamYaw   = 0.0f;
//static float  gCamPitch = 0.0f;
//
//// FPS 统计
//static Uint64 gFpsLast = 0;
//static int    gFpsCount = 0;
//static int    gFps = 0;
//
//// 2D UI: button + label + input
//static jr3d_button_p gBtn = nullptr;
//static jr3d_label_p  gLabel = nullptr;
//static jr3d_input_p  gInput = nullptr;
//static float gMouseX = 0, gMouseY = 0;
//static bool  gMouseDown = false;
//static int   gClickCount = 0;
//
//// -------------------- SDL 回调 --------------------
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
//    (void)argc; (void)argv;
//    *appstate = nullptr;
//
//    // 1) 创建窗口 (jr3d 内部会创建 SDL_GPUDevice)
//    gWin = create_window("texture demo", 1024, 720);
//    if (!gWin) {
//        SDL_Log("create_window failed");
//        return SDL_APP_FAILURE;
//    }
//    gSDLWin = (SDL_Window*)get_raw_window(gWin);
//
//    // 2) 创建 3D 渲染器 (shader 目录: assets/joy_editor/shaders)
//    gR3d = r3d_create(gWin, "joy_editor/shaders");
//    if (!gR3d) {
//        SDL_Log("r3d_create failed");
//        return SDL_APP_FAILURE;
//    }
//
//    // 3) 加载贴图 (正确路径固定为 joy_editor/textures/logo.png, 工作目录为 build/<config>)
//    int w = 0, h = 0;
//    gTex = r3d_load_texture_png(gR3d, "joy_editor/textures/logo.png", &w, &h);
//    if (!gTex) {
//        SDL_Log("No image loaded; will draw solid quad.");
//    }
//
//    // 4) 单位 quad (pos+uv, 范围 [-0.5,0.5])
//    gQuad = r3d_mesh_quad(gR3d);
//    if (!gQuad) {
//        SDL_Log("r3d_mesh_quad failed");
//        return SDL_APP_FAILURE;
//    }
//
//    // 5) 字体初始化 (字体逻辑已封装在 r3d.c: r3d_font_init / r3d_font_draw_fixed)
//    //    优先用 simhei.ttf(黑体, 含中文字形), 回退 arial.ttf(仅 ASCII)
//    if (!r3d_font_init(gR3d, "joy_editor/fonts/simhei.ttf", 48) &&
//        !r3d_font_init(gR3d, "joy_editor/fonts/arial.ttf", 48)) {
//        SDL_Log("r3d_font_init failed (text disabled)");
//    }
//
//    // 6) 2D UI: button + label (屏幕像素坐标, 不随相机变化)
//    gBtn = r3d_button_create(20.0f, 60.0f, 140.0f, 40.0f, "Click Me");
//    r3d_button_set_onclick(gBtn, [](jr3d_button_p /*btn*/) {
//        gClickCount++;
//        SDL_Log("button clicked! count=%d", gClickCount);
//    });
//    gLabel = r3d_label_create(20.0f, 110.0f, 24.0f, 255, 255, 255, "Clicked: 0");
//
//    // 6b) 2D UI: input (单行文本输入, 屏幕像素坐标)
//    gInput = r3d_input_create(20.0f, 160.0f, 260.0f, 36.0f, 22.0f,
//                              30, 30, 30, "type here");
//    r3d_input_set_onsubmit(gInput, [](jr3d_input_p inp) {
//        SDL_Log("input submit: %s", r3d_input_get_text(inp));
//    });
//
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    if (!r3d_begin(gR3d)) return SDL_APP_CONTINUE;
//
//    // 相机: 第三人称俯视, 看向原点
//    vec3_t target = { 0.0f, 0.0f, 0.0f };
//    vec3_t up     = { 0.0f, 1.0f, 0.0f };
//    r3d_camera_lookat(gR3d, gCamPos, target, up, 60.0f);
//
//    // 把 quad 作为一个 2x2 的平面立在世界原点, 贴上图片
//    vec3_t pos   = { 0.0f, 0.0f, 0.0f };
//    vec3_t scale = { 2.0f, 2.0f, 1.0f };
//    if (gTex) {
//        r3d_draw_mesh_tex(gR3d, gQuad, pos, scale, gTex);
//    } else {
//        // 没图就画个纯色 quad, 至少能验证管线
//        jcolor_t c = { 255, 128, 0, 255 };
//        r3d_draw_mesh(gR3d, gQuad, pos, scale, c);
//    }
//
//    // FPS 统计: 每累计 ~0.5s 更新一次
//    {
//        Uint64 now = SDL_GetTicks();
//        gFpsCount++;
//        if (gFpsLast == 0) gFpsLast = now;
//        if (now - gFpsLast >= 500) {
//            gFps = (int)(gFpsCount * 1000 / (now - gFpsLast));
//            gFpsCount = 0;
//            gFpsLast = now;
//        }
//    }
//
//    int winW = 0, winH = 0;
//    r3d_get_window_size(gR3d, &winW, &winH);
//    float uw = (float)(winW > 0 ? winW : 800);
//    float uh = (float)(winH > 0 ? winH : 600);
//
//    // 2D HUD 文字(label): FPS 固定在窗口左上角
//    {
//        char fpsTxt[64];
//        SDL_snprintf(fpsTxt, sizeof(fpsTxt), "FPS: %d", gFps);
//        // 用临时 label 文字更新到 gLabel 之外的 FPS 也行, 这里直接复用 gLabel 之上另画一行:
//        r3d_font_draw_2d(gR3d, gQuad, fpsTxt, 10.0f, 10.0f, 24.0f, uw, uh);
//    }
//
//    // 2D UI label: 显示按钮点击次数
//    if (gLabel) {
//        char buf[64];
//        SDL_snprintf(buf, sizeof(buf), "Clicked: %d", gClickCount);
//        r3d_label_set_text(gLabel, buf);
//        r3d_label_draw(gR3d, gQuad, gLabel, uw, uh);
//    }
//
//    // 2D UI input: 绘制
//    {
//        int winW = 0, winH = 0;
//        r3d_get_window_size(gR3d, &winW, &winH);
//        float iw = (float)(winW > 0 ? winW : 800);
//        float ih = (float)(winH > 0 ? winH : 600);
//        r3d_input_draw(gR3d, gQuad, gInput, iw, ih);
//    }
//
//    // 2D UI button: 先更新状态(基于当前鼠标位置/按下), 再绘制
//    if (gBtn) {
//        r3d_button_is_pressed(gBtn, gMouseDown, gMouseX, gMouseY);
//        int winW = 0, winH = 0;
//        r3d_get_window_size(gR3d, &winW, &winH);
//        r3d_button_draw(gR3d, gQuad, gBtn, (float)(winW > 0 ? winW : 800), (float)(winH > 0 ? winH : 600));
//    }
//
//    r3d_end(gR3d);
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    (void)appstate;
//    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
//
//    // 鼠标位置/按下状态(供 2D UI button 使用)
//    if (event->type == SDL_EVENT_MOUSE_MOTION) {
//        gMouseX = (float)event->motion.x;
//        gMouseY = (float)event->motion.y;
//    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
//        gMouseX = (float)event->button.x;
//        gMouseY = (float)event->button.y;
//        gMouseDown = true;
//        // 若左键按在 button 上, 交给 UI 处理, 不旋转相机
//        if (gBtn && r3d_button_is_hovered(gBtn, gMouseX, gMouseY)) {
//            return SDL_APP_CONTINUE;
//        }
//    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
//        gMouseDown = false;
//        if (gBtn) r3d_button_release(gBtn, gMouseX, gMouseY);
//    }
//
//    // input 统一事件处理 (对齐 jr2d inputbox_process_event, 见 ui_test.cpp)
//    // 内部处理: 鼠标聚焦/定位光标、文本输入、键盘(退格/删除/方向/Home/End/
//    // Ctrl+A/C/V/X/回车)。点击输入框内部时返回 true(事件被消费, 不旋转相机)。
//    bool input_consumed = false;
//    if (gInput) {
//        input_consumed = r3d_input_handle_event(gR3d, gInput, event);
//    }
//
//    // 鼠标拖拽旋转相机 (仅当事件未被 input 消费时)
//    static bool dragging = false;
//    static float lastX = 0, lastY = 0;
//    if (!input_consumed) {
//        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
//            dragging = true; lastX = event->button.x; lastY = event->button.y;
//        } else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
//            dragging = false;
//        } else if (event->type == SDL_EVENT_MOUSE_MOTION && dragging) {
//            float dx = event->motion.x - lastX;
//            float dy = event->motion.y - lastY;
//            lastX = event->motion.x; lastY = event->motion.y;
//            gCamYaw   += dx * 0.01f;
//            gCamPitch += dy * 0.01f;
//            if (gCamPitch > 1.5f) gCamPitch = 1.5f;
//            if (gCamPitch < -1.5f) gCamPitch = -1.5f;
//            // 重新计算相机位置 (绕原点转)
//            float r = 4.0f;
//            gCamPos.x = r * cosf(gCamPitch) * sinf(gCamYaw);
//            gCamPos.y = 1.0f + r * sinf(gCamPitch);
//            gCamPos.z = r * cosf(gCamPitch) * cosf(gCamYaw);
//        }
//    }
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate; (void)result;
//    if (gQuad) r3d_mesh_destroy(gR3d, gQuad);
//    if (gBtn)  r3d_button_destroy(gBtn);
//    if (gLabel) r3d_label_destroy(gLabel);
//    if (gInput) r3d_input_destroy(gInput);
//    // gTex 由 r3d 内部管理, 随 r3d_destroy 释放(字体纹理同理)
//    if (gR3d) r3d_destroy(gR3d);
//    if (gWin)  destroy_window(gWin);
//    SDL_Quit();
//}
