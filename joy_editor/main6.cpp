//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//
//// 平台特定的 OpenGL 头文件
//#ifdef __EMSCRIPTEN__
//#include <GLES2/gl2.h>
//#else
//#include "glad.h"
//#endif
//
//#include <math.h>
//
//// 全局窗口
//static SDL_Window* g_window = NULL;
//static SDL_GLContext g_gl_ctx = NULL;
//
//// 着色器 & 资源
//static GLuint g_prog = 0;
//static GLuint g_vbo = 0, g_ebo = 0;
//static GLint aPosLoc, aColorLoc;
//static GLint uModelLoc, uViewLoc, uProjLoc;
//
//// 窗口尺寸（用于适配投影矩阵）
//static int g_windowWidth = 800;
//static int g_windowHeight = 600;
//static Uint64 g_lastTime = 0;    // 上一帧时间（纳秒）
//
//// 新增：逻辑分辨率（固定渲染尺寸）
//static const int LOGIC_W = 800;
//static const int LOGIC_H = 600;
//// 实际窗口拉伸后的渲染区域（用于居中/适配）
//static int g_renderX, g_renderY, g_renderW, g_renderH;
//
//// ============ 矩阵工具 ============
//static void MatIdentity(float m[16]) {
//    for (int i = 0; i < 16; i++) m[i] = 0;
//    m[0] = m[5] = m[10] = m[15] = 1.0f;
//}
//
//static void MatRotateX(float out[16], float rad) {
//    MatIdentity(out);
//    float c = cosf(rad), s = sinf(rad);
//    out[5] = c; out[6] = s;
//    out[9] = -s; out[10] = c;
//}
//
//static void MatRotateY(float out[16], float rad) {
//    MatIdentity(out);
//    float c = cosf(rad), s = sinf(rad);
//    out[0] = c; out[2] = s;
//    out[8] = -s; out[10] = c;
//}
//
//static void MatMul(float out[16], const float a[16], const float b[16]) {
//    float res[16];
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            res[i * 4 + j] =
//                a[i * 4 + 0] * b[0 * 4 + j] +
//                a[i * 4 + 1] * b[1 * 4 + j] +
//                a[i * 4 + 2] * b[2 * 4 + j] +
//                a[i * 4 + 3] * b[3 * 4 + j];
//        }
//    }
//    for (int i = 0; i < 16; i++) out[i] = res[i];
//}
//
//static void MatLookAt(float view[16], float eyeX, float eyeY, float eyeZ, float cenX, float cenY, float cenZ, float upX, float upY, float upZ) {
//    float fx = cenX - eyeX, fy = cenY - eyeY, fz = cenZ - eyeZ;
//    float len = sqrtf(fx * fx + fy * fy + fz * fz); fx /= len; fy /= len; fz /= len;
//    float rx = fy * upZ - fz * upY, ry = fz * upX - fx * upZ, rz = fx * upY - fy * upX;
//    len = sqrtf(rx * rx + ry * ry + rz * rz); rx /= len; ry /= len; rz /= len;
//    float ux = ry * fz - rz * fy, uy = rz * fx - rx * fz, uz = rx * fy - ry * fx;
//    MatIdentity(view);
//    view[0] = rx; view[1] = ux; view[2] = -fx;
//    view[4] = ry; view[5] = uy; view[6] = -fy;
//    view[8] = rz; view[9] = uz; view[10] = -fz;
//    view[12] = -(rx * eyeX + ry * eyeY + rz * eyeZ);
//    view[13] = -(ux * eyeX + uy * eyeY + uz * eyeZ);
//    view[14] = -(-fx * eyeX - fy * eyeY - fz * eyeZ);
//}
//
//// 修复后的透视投影矩阵
//static void MatPerspective(float proj[16], float fov, float aspect, float near, float far) {
//    MatIdentity(proj);
//    float tanHalfFovy = tanf(fov / 2.0f);
//    
//    proj[0]  = 1.0f / (aspect * tanHalfFovy);  // X轴缩放
//    proj[5]  = 1.0f / tanHalfFovy;              // Y轴缩放
//    proj[10] = -(far + near) / (far - near);    // Z轴深度映射
//    proj[11] = -1.0f;                           // 齐次坐标
//    proj[14] = -(2.0f * far * near) / (far - near);
//    proj[15] = 0.0f;
//}
//
//// 新增：计算逻辑分辨率在实际窗口中的拉伸/居中区域（保持宽高比）
//static void CalcRenderRect(int windowW, int windowH,
//                           int logicW, int logicH,
//                           int* outX, int* outY, int* outW, int* outH) {
//    float winAspect = (float)windowW / windowH;
//    float logicAspect = (float)logicW / logicH;
//
//    if (winAspect > logicAspect) {
//        // 窗口更宽：按高度适配，左右留黑边
//        *outH = windowH;
//        *outW = (int)(windowH * logicAspect);
//        *outX = (windowW - *outW) / 2;
//        *outY = 0;
//    } else {
//        // 窗口更高/手机竖屏：按宽度适配，上下留黑边
//        *outW = windowW;
//        *outH = (int)(windowW / logicAspect);
//        *outX = 0;
//        *outY = (windowH - *outH) / 2;
//    }
//}
//
//// ============ 立方体顶点与颜色 ============
//static const float cubeVerts[] = {
//    -1,-1,-1,  1,0,0,
//     1,-1,-1,  0,1,0,
//     1, 1,-1,  1,1,0,
//    -1, 1,-1,  0,0,1,
//    -1,-1, 1,  1,0,1,
//     1,-1, 1,  0,1,1,
//     1, 1, 1,  1,1,1,
//    -1, 1, 1,  0,0,0,
//};
//
//static const GLuint cubeIdx[] = {
//    0,1,2, 0,2,3,
//    4,5,6, 4,6,7,
//    0,1,5, 0,5,4,
//    3,2,6, 3,6,7,
//    1,2,6, 1,6,5,
//    0,3,7, 0,7,4
//};
//
//// ============ 平台兼容的着色器 ============
//#if defined(__EMSCRIPTEN__) || defined(ANDROID)
//// OpenGL ES 2.0 着色器 (Web/Android)
//static const char* vShaderSrc =
//"attribute vec3 aPos;\n"
//"attribute vec3 aColor;\n"
//"uniform mat4 uModel,uView,uProj;\n"
//"varying vec3 vCol;\n"
//"void main(){\n"
//"    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);\n"
//"    vCol = aColor;\n"
//"}";
//
//static const char* fShaderSrc =
//"precision mediump float;\n"
//"varying vec3 vCol;\n"
//"void main(){ gl_FragColor=vec4(vCol,1.0); }";
//#else
//// OpenGL 3.3 着色器 (Windows/Linux/macOS)
//static const char* vShaderSrc =
//"#version 330 core\n"
//"layout(location = 0) in vec3 aPos;\n"
//"layout(location = 1) in vec3 aColor;\n"
//"uniform mat4 uModel,uView,uProj;\n"
//"out vec3 vCol;\n"
//"void main(){\n"
//"    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);\n"
//"    vCol = aColor;\n"
//"}";
//
//static const char* fShaderSrc =
//"#version 330 core\n"
//"in vec3 vCol;\n"
//"out vec4 FragColor;\n"
//"void main(){ FragColor=vec4(vCol,1.0); }";
//#endif
//
//static GLuint CompileShader(GLenum type, const char* src) {
//    GLuint s = glCreateShader(type);
//    glShaderSource(s, 1, &src, NULL);
//    glCompileShader(s);
//
//    // 编译错误检查
//    GLint success;
//    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        char infoLog[512];
//        glGetShaderInfoLog(s, 512, NULL, infoLog);
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader Error: %s", infoLog);
//    }
//    return s;
//}
//
//static int InitShader(void) {
//    GLuint vs = CompileShader(GL_VERTEX_SHADER, vShaderSrc);
//    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fShaderSrc);
//    g_prog = glCreateProgram();
//    glAttachShader(g_prog, vs);
//    glAttachShader(g_prog, fs);
//    glLinkProgram(g_prog);
//
//    // 链接错误检查
//    GLint success;
//    glGetProgramiv(g_prog, GL_LINK_STATUS, &success);
//    if (!success) {
//        char infoLog[512];
//        glGetProgramInfoLog(g_prog, 512, NULL, infoLog);
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program Link Error: %s", infoLog);
//    }
//
//    glDeleteShader(vs);
//    glDeleteShader(fs);
//
//    // 获取属性位置
//#if defined(__EMSCRIPTEN__) || defined(ANDROID)
//    aPosLoc = glGetAttribLocation(g_prog, "aPos");
//    aColorLoc = glGetAttribLocation(g_prog, "aColor");
//#else
//    aPosLoc = 0; // 对应 layout(location = 0)
//    aColorLoc = 1; // 对应 layout(location = 1)
//#endif
//
//    uModelLoc = glGetUniformLocation(g_prog, "uModel");
//    uViewLoc = glGetUniformLocation(g_prog, "uView");
//    uProjLoc = glGetUniformLocation(g_prog, "uProj");
//    return 1;
//}
//
//static int InitCubeBuffer(void) {
//    glGenBuffers(1, &g_vbo);
//    glGenBuffers(1, &g_ebo);
//
//    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);
//    return 1;
//}
//
//// ============ SDL 主逻辑 ============
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
//{
//    (void)argc; (void)argv; *appstate = NULL;
//
//    // 1. 平台特定的 OpenGL 配置
//#ifdef _WIN32
//    // Windows: 请求 OpenGL 3.3 Core Profile
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//#elif defined(__EMSCRIPTEN__) || defined(ANDROID)
//    // Web/Android: 请求 OpenGL ES 2.0
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//#endif
//
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        SDL_Log("SDL init err: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    g_window = SDL_CreateWindow("SDL3 Cross-Platform Cube", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
//    if (!g_window) {
//        SDL_Log("Create window err: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    g_gl_ctx = SDL_GL_CreateContext(g_window);
//    if (!g_gl_ctx) {
//        SDL_Log("Create GL context err: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    // 2. 平台特定的交换间隔
//#ifdef _WIN32
//    SDL_GL_SetSwapInterval(1); // Windows 开启垂直同步防止撕裂
//#endif
//
//    // 3. 加载 OpenGL 函数指针
//#ifndef __EMSCRIPTEN__
//    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load GLAD");
//        return SDL_APP_FAILURE;
//    }
//#endif
//
//    InitShader();
//    InitCubeBuffer();
//
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
//
//    // 初始化渲染区域
//    CalcRenderRect(g_windowWidth, g_windowHeight,
//                   LOGIC_W, LOGIC_H,
//                   &g_renderX, &g_renderY, &g_renderW, &g_renderH);
//    // 视口设置为拉伸后的区域（而非整个窗口）
//    glViewport(g_renderX, g_renderY, g_renderW, g_renderH);
//
//    glUseProgram(g_prog);
//    glVertexAttribPointer(aPosLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(aPosLoc);
//    glVertexAttribPointer(aColorLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(aColorLoc);
//    // 新增：初始化时间戳
//    g_lastTime = SDL_GetTicksNS();
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* e) {
//    (void)appstate;
//    if (e->type == SDL_EVENT_QUIT)
//        return SDL_APP_SUCCESS;
//
//    // 窗口大小变化处理 (全平台)
//    if (e->type == SDL_EVENT_WINDOW_RESIZED) {
//        g_windowWidth = e->window.data1;
//        g_windowHeight = e->window.data2;
//        // 重新计算渲染区域
//        CalcRenderRect(g_windowWidth, g_windowHeight,
//                       LOGIC_W, LOGIC_H,
//                       &g_renderX, &g_renderY, &g_renderW, &g_renderH);
//        // 更新视口到新的拉伸区域
//        glViewport(g_renderX, g_renderY, g_renderW, g_renderH);
//    }
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate) {
//    (void)appstate;
//
//    // ============ 修改：基于时间的旋转速度（跨平台统一60帧流畅度） ============
//    Uint64 now = SDL_GetTicksNS();                // 获取当前纳秒时间
//    float deltaTime = (now - g_lastTime) / 1e9f; // 计算上一帧耗时（秒）
//    g_lastTime = now;                            // 更新上一帧时间
//
//    static float angle = 0.0f;
//    angle += deltaTime * 0.9f;
//
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    float model[16], view[16], proj[16];
//    MatIdentity(model);
//
//    float rx = angle * 0.8f;
//    float ry = angle * 0.5f;
//    float mx[16], my[16];
//    MatRotateX(mx, rx);
//    MatRotateY(my, ry);
//
//    MatMul(model, mx, my);
//
//    MatLookAt(view, 0, 0, 5, 0, 0, 0, 0, 1, 0);
//
//    // 固定逻辑分辨率的宽高比（不再随窗口变化）
//    float aspect = (float)LOGIC_W / LOGIC_H;
//    MatPerspective(proj, 3.14159f / 3.0f, aspect, 0.1f, 100.0f);
//
//    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, model);
//    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, view);
//    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, proj);
//
//    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
//    SDL_GL_SwapWindow(g_window);
//
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult res) {
//    (void)appstate; (void)res;
//    if (g_vbo) glDeleteBuffers(1, &g_vbo);
//    if (g_ebo) glDeleteBuffers(1, &g_ebo);
//    if (g_prog) glDeleteProgram(g_prog);
//    SDL_GL_DestroyContext(g_gl_ctx);
//    SDL_DestroyWindow(g_window);
//    SDL_Quit();
//}