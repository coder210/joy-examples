//// ui_test.cpp — JUI 组件交互式测试（C++ 版）
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <cmath>
//#include <string>
//#include <joy/joy.h>
//
//// ======================== 常量 ========================
//static const int WINDOW_W = 800, WINDOW_H = 600;
//static jwindow_p gWindow = nullptr;
//static jr2d_p    gRender = nullptr;
//static font_p    gFont   = nullptr;
//static bool      gRunning = true;
//
//// ======================== JUI 组件 ========================
//static label_p    gLabel    = nullptr;
//static inputbox_p gInput    = nullptr;
//static button_p   gBtn1     = nullptr;
//static button_p   gBtn2     = nullptr;
//static checkbox_p gCb       = nullptr;
//static combobox_p gCb2      = nullptr;
//static slider_p   gSlider   = nullptr;
//static popup_p    gPopup    = nullptr;
//static tooltip_p  gTip      = nullptr;
//static joystick_p gJoystick = nullptr;
//
//
//
//// ======================== 初始化 ========================
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
//    (void)argc; (void)argv;
//    *appstate = nullptr;
//
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        SDL_Log("SDL_Init: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    gWindow = create_window("JUI Test", WINDOW_W, WINDOW_H);
//    if (!gWindow) {
//        SDL_Log("create_window failed");
//        return SDL_APP_FAILURE;
//    }
//    gRender = r2d_create(gWindow);
//    if (!gRender) {
//        SDL_Log("r2d_create failed");
//        return SDL_APP_FAILURE;
//    }
//
//    gFont = font_create("joy_editor/fonts/simhei.ttf", 24);
//    if (!gFont) {
//        gFont = font_create("joy_beat/fonts/simhei.ttf", 24);
//    }
//    if (!gFont) {
//        SDL_Log("font_create failed");
//    }
//
//    // ---- Label ----
//    gLabel = label_create(gRender, gFont, NULL, 0);
//
//    // ---- Inputbox ----
//    {
//        jrect_t rect = {10, 35, 250, 30};
//        gInput = inputbox_create(gRender, rect, gFont);
//        inputbox_set_text(gInput, "hello jui");
//    }
//
//    // ---- Buttons ----
//    gBtn1 = button_create(gRender, 120, 30);
//    button_set_position(gBtn1, 10, 80);
//    if (gFont) {
//        jcolor_t white = {255, 255, 255, 255};
//        button_set_textx(gBtn1, gFont, "Button1", 7, white);
//    }
//
//    gBtn2 = button_create(gRender, 120, 30);
//    button_set_position(gBtn2, 150, 80);
//    if (gFont) {
//        jcolor_t white = {255, 255, 255, 255};
//        button_set_textx(gBtn2, gFont, "Button2", 7, white);
//    }
//
//    // ---- Checkbox ----
//    gCb = checkbox_create(10, 130, 20, NULL);
//
//    // ---- Combobox ----
//    {
//        jrect_t rect = {120, 125, 140, 28};
//        gCb2 = combobox_create(gRender, rect);
//        if (gFont) {
//            jcolor_t black = {0,0,0,255};
//            combobox_add_item(gCb2, gFont, "1", 1, black);
//            combobox_add_item(gCb2, gFont, "2", 1, black);
//            combobox_add_item(gCb2, gFont, "3", 1, black);
//        }
//    }
//
//    // ---- Slider ----
//    gSlider = slider_create(gRender, 10, 170, 200, 16, 0, 100, 50);
//
//    // ---- Popup ----
//    gPopup = popup_create(gRender, 350, 220);
//    popup_set_title(gPopup, "Popup Demo", 10);
//    popup_set_content(gPopup, "This is a popup dialog.\nYou can drag it by the title bar.\nPress P to toggle visibility.\nClose with the X button or press P again.", -1);
//    popup_set_position(gPopup, 225, 170);
//
//    // ---- Tooltip ----
//    gTip = tooltip_create(gRender, "hover me!", 9);
//
//    // ---- Joystick ----
//    gJoystick = joystick_create(gRender, 650, 450, 80);
//
//    SDL_Log("JUI Test initialized");
//    return SDL_APP_CONTINUE;
//}
//
//// ======================== 事件（全权交给 process_event）===================
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
//    (void)appstate;
//
//    if (event->type == SDL_EVENT_QUIT) {
//        gRunning = false;
//        return SDL_APP_SUCCESS;
//    }
//
//    if (event->type == SDL_EVENT_KEY_DOWN) {
//        if (event->key.scancode == SDL_SCANCODE_ESCAPE ||
//            event->key.scancode == SDL_SCANCODE_Q) {
//            gRunning = false;
//            return SDL_APP_SUCCESS;
//        }
//        if (event->key.scancode == SDL_SCANCODE_P) {
//            // P 键切换 popup 可见性（当前可见性由 popup_process_event 的关闭按钮控制）
//            // 这里简单取反：每次 P 键切换一次
//            static bool popup_p_visible = true;
//            popup_p_visible = !popup_p_visible;
//            if (gPopup) popup_set_visible(gPopup, popup_p_visible);
//        }
//    }
//
//    // 所有组件的事件统一交给 process_event 处理
//    if (gLabel)    label_process_event(gLabel, event);
//    if (gInput)    inputbox_process_event(gInput, event);
//    if (gBtn1)     button_process_event(gBtn1, event);
//    if (gBtn2)     button_process_event(gBtn2, event);
//    if (gCb)       checkbox_process_event(gCb, event);
//    if (gCb2)      combobox_process_event(gCb2, event);
//    if (gSlider)   slider_process_event(gSlider, event);
//    if (gPopup)    popup_process_event(gPopup, event);
//    if (gTip)      tooltip_process_event(gTip, event);
//    if (gJoystick) joystick_process_event(gJoystick, event);
//
//    return SDL_APP_CONTINUE;
//}
//
//// ======================== 渲染 ========================
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    r2d_set_color(gRender, 50, 50, 50, 255);
//    r2d_begin(gRender);
//
//    if (gLabel) label_draw(gLabel);
//    if (gInput) inputbox_draw(gInput);
//    if (gBtn1) button_draw(gBtn1);
//    if (gBtn2) button_draw(gBtn2);
//    if (gCb) checkbox_draw(gCb, gRender, gFont);
//    if (gCb2) combobox_draw(gCb2);
//    if (gSlider) slider_draw(gSlider);
//
//    if (gFont) {
//        jcolor_t white = {255, 255, 255, 255};
//        char buf[256];
//        snprintf(buf, sizeof(buf), "text: %s", gInput ? inputbox_get_text(gInput) : "");
//        jtext_printx(gRender, gFont, buf, (int)strlen(buf), white, 10, 450);
//        if (gSlider) {
//            snprintf(buf, sizeof(buf), "slider: %.0f", slider_get_value(gSlider));
//            jtext_printx(gRender, gFont, buf, (int)strlen(buf), white, 10, 510);
//        }
//    }
//
//    if (gPopup) popup_draw(gPopup, gFont, gFont);
//    if (gTip) tooltip_draw(gTip, gFont);
//
//    if (gJoystick) {
//        joystick_draw(gJoystick);
//        vec2_t dir = joystick_get_direction(gJoystick);
//        float mag = joystick_get_magnitude(gJoystick);
//        if (gFont) {
//            char buf[80];
//            snprintf(buf, sizeof(buf), "Dir: (%.2f, %.2f) Mag: %.2f", dir.x, dir.y, mag);
//            jcolor_t cyan = {200, 200, 255, 255};
//            jtext_printx(gRender, gFont, buf, (int)strlen(buf), cyan, 600, 540);
//        }
//    }
//
//    r2d_end(gRender);
//    return SDL_APP_CONTINUE;
//}
//
//// ======================== 清理 ========================
//void SDL_AppQuit(void* appstate, SDL_AppResult result) {
//    (void)appstate; (void)result;
//
//    if (gJoystick) joystick_destroy(gJoystick);
//    if (gTip) tooltip_destroy(gTip);
//    if (gPopup) popup_destroy(gPopup);
//    if (gSlider) slider_destroy(gSlider);
//    if (gCb2) combobox_destroy(gCb2);
//    if (gCb) checkbox_destroy(gCb);
//    if (gBtn1) button_destroy(gBtn1);
//    if (gBtn2) button_destroy(gBtn2);
//    if (gInput) inputbox_destroy(gInput);
//    if (gLabel) label_destroy(gLabel);
//    if (gFont) font_destroy(gFont);
//    if (gRender) r2d_destroy(gRender);
//    if (gWindow) destroy_window(gWindow);
//
//    SDL_Log("JUI Test quit");
//}
