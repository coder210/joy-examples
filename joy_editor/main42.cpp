//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <SDL3/SDL_opengles2.h>
//
//#include <math.h>
//
//// ============ 全部GLES函数指针 ============
//static PFNGLCLEARCOLORPROC               glClearColor2 = NULL;
//static PFNGLCLEARPROC                    glClear2 = NULL;
//static PFNGLENABLEPROC                   glEnable2 = NULL;
//static PFNGLDEPTHFUNCPROC                glDepthFunc2 = NULL;
//
//static PFNGLGENBUFFERSPROC               glGenBuffers2 = NULL;
//static PFNGLBINDBUFFERPROC               glBindBuffer2 = NULL;
//static PFNGLBUFFERDATAPROC               glBufferData2 = NULL;
//
//static PFNGLCREATESHADERPROC             glCreateShader2 = NULL;
//static PFNGLSHADERSOURCEPROC             glShaderSource2 = NULL;
//static PFNGLCOMPILESHADERPROC            glCompileShader2 = NULL;
//static PFNGLCREATEPROGRAMPROC            glCreateProgram2 = NULL;
//static PFNGLATTACHSHADERPROC             glAttachShader2 = NULL;
//static PFNGLLINKPROGRAMPROC              glLinkProgram2 = NULL;
//static PFNGLUSEPROGRAMPROC               glUseProgram2 = NULL;
//
//static PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation2 = NULL;
//static PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray2 = NULL;
//static PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer2 = NULL;
//
//static PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation2 = NULL;
//static PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv2 = NULL;
//
//static PFNGLDRAWELEMENTSPROC             glDrawElements2 = NULL;
//static PFNGLDELETEBUFFERSPROC            glDeleteBuffers2 = NULL;
//static PFNGLDELETESHADERPROC             glDeleteShader2 = NULL;
//static PFNGLDELETEPROGRAMPROC            glDeleteProgram2 = NULL;
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
//// ============ 矩阵工具 ============
//static void MatIdentity(float m[16]) {
//        for (int i = 0; i < 16; i++) m[i] = 0;
//        m[0] = m[5] = m[10] = m[15] = 1.0f;
//}
//
//static void MatRotateX(float out[16], float rad) {
//        MatIdentity(out);
//        float c = cosf(rad), s = sinf(rad);
//        out[5] = c; out[6] = s;
//        out[9] = -s; out[10] = c;
//}
//
//static void MatRotateY(float out[16], float rad) {
//        MatIdentity(out);
//        float c = cosf(rad), s = sinf(rad);
//        out[0] = c; out[2] = s;
//        out[8] = -s; out[10] = c;
//}
//
//static void MatMul(float out[16], const float a[16], const float b[16]) {
//        float res[16];
//        for (int i = 0; i < 4; i++) {
//                for (int j = 0; j < 4; j++) {
//                        res[i * 4 + j] =
//                                a[i * 4 + 0] * b[0 * 4 + j] +
//                                a[i * 4 + 1] * b[1 * 4 + j] +
//                                a[i * 4 + 2] * b[2 * 4 + j] +
//                                a[i * 4 + 3] * b[3 * 4 + j];
//                }
//        }
//        for (int i = 0; i < 16; i++) out[i] = res[i];
//}
//
//static void MatLookAt(float view[16], float eyeX, float eyeY, float eyeZ, float cenX, float cenY, float cenZ, float upX, float upY, float upZ) {
//        float fx = cenX - eyeX, fy = cenY - eyeY, fz = cenZ - eyeZ;
//        float len = sqrtf(fx * fx + fy * fy + fz * fz); fx /= len; fy /= len; fz /= len;
//        float rx = fy * upZ - fz * upY, ry = fz * upX - fx * upZ, rz = fx * upY - fy * upX;
//        len = sqrtf(rx * rx + ry * ry + rz * rz); rx /= len; ry /= len; rz /= len;
//        float ux = ry * fz - rz * fy, uy = rz * fx - rx * fz, uz = rx * fy - ry * fx;
//        MatIdentity(view);
//        view[0] = rx; view[1] = ux; view[2] = -fx;
//        view[4] = ry; view[5] = uy; view[6] = -fy;
//        view[8] = rz; view[9] = uz; view[10] = -fz;
//        view[12] = -(rx * eyeX + ry * eyeY + rz * eyeZ);
//        view[13] = -(ux * eyeX + uy * eyeY + uz * eyeZ);
//        view[14] = -(-fx * eyeX - fy * eyeY - fz * eyeZ);
//}
//
//static void MatPerspective(float proj[16], float fov, float aspect, float near, float far) {
//        MatIdentity(proj);
//        float tanH = tanf(fov / 2.0f);
//        proj[0] = 1.0f / (aspect * tanH);
//        proj[5] = 1.0f / tanH;
//        proj[10] = -(far + near) / (far - near);
//        proj[11] = -1.0f;
//        proj[14] = -(2.0f * far * near) / (far - near);
//        proj[15] = 0.0f;
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
//// ============ 兼容 WebGL 的着色器 ============
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
//
//// ============ 逐行加载函数 ============
//static int LoadGLES2Functions(void)
//{
//        glClearColor2 = (PFNGLCLEARCOLORPROC)SDL_GL_GetProcAddress("glClearColor");
//        glClear2 = (PFNGLCLEARPROC)SDL_GL_GetProcAddress("glClear");
//        glEnable2 = (PFNGLENABLEPROC)SDL_GL_GetProcAddress("glEnable");
//        glDepthFunc2 = (PFNGLDEPTHFUNCPROC)SDL_GL_GetProcAddress("glDepthFunc");
//
//        glGenBuffers2 = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
//        glBindBuffer2 = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
//        glBufferData2 = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
//
//        glCreateShader2 = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
//        glShaderSource2 = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
//        glCompileShader2 = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
//
//        glCreateProgram2 = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
//        glAttachShader2 = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
//        glLinkProgram2 = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
//        glUseProgram2 = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
//
//        glGetAttribLocation2 = (PFNGLGETATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glGetAttribLocation");
//        glEnableVertexAttribArray2 = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
//        glVertexAttribPointer2 = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
//
//        glGetUniformLocation2 = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
//        glUniformMatrix4fv2 = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
//
//        glDrawElements2 = (PFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");
//        glDeleteBuffers2 = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
//        glDeleteShader2 = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
//        glDeleteProgram2 = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
//
//        if (!glClearColor2 || !glClear2 || !glEnable2 || !glDepthFunc2 ||
//                !glGenBuffers2 || !glBindBuffer2 || !glBufferData2 ||
//                !glCreateShader2 || !glShaderSource2 || !glCompileShader2 ||
//                !glCreateProgram2 || !glAttachShader2 || !glLinkProgram2 || !glUseProgram2 ||
//                !glGetAttribLocation2 || !glEnableVertexAttribArray2 || !glVertexAttribPointer2 ||
//                !glGetUniformLocation2 || !glUniformMatrix4fv2 ||
//                !glDrawElements2 || !glDeleteBuffers2 || !glDeleteShader2 || !glDeleteProgram2)
//        {
//                SDL_Log("Failed to load GLES2 functions");
//                return -1;
//        }
//        return 0;
//}
//
//static GLuint CompileShader(GLenum type, const char* src) {
//        GLuint s = glCreateShader2(type);
//        glShaderSource2(s, 1, &src, NULL);
//        glCompileShader2(s);
//        return s;
//}
//
//static int InitShader(void) {
//        GLuint vs = CompileShader(GL_VERTEX_SHADER, vShaderSrc);
//        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fShaderSrc);
//        g_prog = glCreateProgram2();
//        glAttachShader2(g_prog, vs);
//        glAttachShader2(g_prog, fs);
//        glLinkProgram2(g_prog);
//        glDeleteShader2(vs);
//        glDeleteShader2(fs);
//
//        aPosLoc = glGetAttribLocation2(g_prog, "aPos");
//        aColorLoc = glGetAttribLocation2(g_prog, "aColor");
//        uModelLoc = glGetUniformLocation2(g_prog, "uModel");
//        uViewLoc = glGetUniformLocation2(g_prog, "uView");
//        uProjLoc = glGetUniformLocation2(g_prog, "uProj");
//        return 1;
//}
//
//static int InitCubeBuffer(void) {
//        glGenBuffers2(1, &g_vbo);
//        glGenBuffers2(1, &g_ebo);
//
//        glBindBuffer2(GL_ARRAY_BUFFER, g_vbo);
//        glBufferData2(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
//
//        glBindBuffer2(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
//        glBufferData2(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);
//        return 1;
//}
//
//// ============ SDL 主逻辑 ============
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
//{
//        (void)argc; (void)argv; *appstate = NULL;
//        if (!SDL_Init(SDL_INIT_VIDEO)) {
//                SDL_Log("init err: %s", SDL_GetError());
//                return SDL_APP_FAILURE;
//        }
//
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//
//        g_window = SDL_CreateWindow("SDL3 GLES2 Cube", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
//        if (!g_window)return SDL_APP_FAILURE;
//        g_gl_ctx = SDL_GL_CreateContext(g_window);
//        if (!g_gl_ctx)return SDL_APP_FAILURE;
//
//        if (LoadGLES2Functions() < 0)return SDL_APP_FAILURE;
//        InitShader();
//        InitCubeBuffer();
//
//        glEnable2(GL_DEPTH_TEST);
//        glDepthFunc2(GL_LESS);
//        glClearColor2(0.15f, 0.15f, 0.2f, 1.0f);
//
//        glUseProgram2(g_prog);
//        glVertexAttribPointer2(aPosLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray2(aPosLoc);
//        glVertexAttribPointer2(aColorLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray2(aColorLoc);
//
//        return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* e) {
//        (void)appstate;
//        if (e->type == SDL_EVENT_QUIT)return SDL_APP_SUCCESS;
//        return SDL_APP_CONTINUE;
//}
//
//// ====================== 修复：旋转逻辑 100% 和你参考代码一致 ======================
//SDL_AppResult SDL_AppIterate(void* appstate) {
//        (void)appstate;
//        static float angle = 0.0f;
//        angle += 0.015f;
//
//        glClear2(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        float model[16], view[16], proj[16];
//        MatIdentity(model);
//
//        // 正确旋转：先 X 后 Y，矩阵顺序正确
//        float rx = angle * 0.8f;
//        float ry = angle * 0.5f;
//        float mx[16], my[16];
//        MatRotateX(mx, rx);
//        MatRotateY(my, ry);
//
//        // 修复：先乘X，再乘Y → 和你参考代码完全一致
//        MatMul(model, mx, my);
//
//        MatLookAt(view, 0, 0, 5, 0, 0, 0, 0, 1, 0);
//        MatPerspective(proj, 3.14159f / 3.0f, 800.0f / 600.0f, 0.1f, 100.0f);
//
//        glUniformMatrix4fv2(uModelLoc, 1, GL_FALSE, model);
//        glUniformMatrix4fv2(uViewLoc, 1, GL_FALSE, view);
//        glUniformMatrix4fv2(uProjLoc, 1, GL_FALSE, proj);
//
//        glDrawElements2(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
//        SDL_GL_SwapWindow(g_window);
//        return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult res) {
//        (void)appstate; (void)res;
//        if (g_vbo)glDeleteBuffers2(1, &g_vbo);
//        if (g_ebo)glDeleteBuffers2(1, &g_ebo);
//        if (g_prog)glDeleteProgram2(g_prog);
//        SDL_GL_DestroyContext(g_gl_ctx);
//        SDL_DestroyWindow(g_window);
//        SDL_Quit();
//}