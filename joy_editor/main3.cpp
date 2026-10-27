//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//#include <stdio.h>
//#include <math.h>
//#include <stdlib.h>  // 修复 qsort 未定义
//
//#define SCREEN_W 800
//#define SCREEN_H 600
//
//static SDL_Window* window = NULL;
//static SDL_Renderer* renderer = NULL;
//
//typedef struct { float x, y, z; } Vec3;
//
//typedef struct {
//        float z;
//        int face;
//} FaceDepth;
//
//Vec3 cubeRaw[8] = {
//    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
//    {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
//};
//
//int faceList[6][4] = {
//    {0,1,2,3}, {4,5,6,7},
//    {0,1,5,4}, {3,2,6,7},
//    {1,5,6,2}, {0,4,7,3}
//};
//
//float colors[6][4] = {
//    {1,0,0,1}, {0,1,0,1},
//    {0,0,1,1}, {1,1,0,1},
//    {1,0,1,1}, {0,1,1,1}
//};
//
//static Uint64 startTick = 0;
//
//static Vec3 Project(Vec3 p, float time, float* outRawZ)
//{
//        float rx = time * 0.8f;
//        float ry = time * 0.5f;
//
//        float y1 = p.y * cosf(rx) - p.z * sinf(rx);
//        float z1 = p.y * sinf(rx) + p.z * cosf(rx);
//        p.y = y1;
//        p.z = z1;
//
//        float x1 = p.x * cosf(ry) + p.z * sinf(ry);
//        float zz = -p.x * sinf(ry) + p.z * cosf(ry);
//        p.x = x1;
//        p.z = zz;
//
//        *outRawZ = p.z;
//
//        float scale = 200.0f / (p.z + 4.0f);
//        Vec3 res;
//        res.x = p.x * scale + SCREEN_W / 2.0f;
//        res.y = p.y * scale + SCREEN_H / 2.0f;
//        res.z = 0;
//        return res;
//}
//
//// 排序函数
//int SortDepth(const void* a, const void* b) {
//        FaceDepth* fa = (FaceDepth*)a;
//        FaceDepth* fb = (FaceDepth*)b;
//        if (fa->z > fb->z) return -1;
//        return 1;
//}
//
////=======================================================
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
//{
//        SDL_SetAppMetadata("3D Cube", "1.0", "com.example");
//
//        if (!SDL_Init(SDL_INIT_VIDEO)) {
//                return SDL_APP_FAILURE;
//        }
//
//        window = SDL_CreateWindow("3D Cube", SCREEN_W, SCREEN_H, 0);
//        renderer = SDL_CreateRenderer(window, NULL);
//
//        startTick = SDL_GetTicks();
//        return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
//{
//        if (event->type == SDL_EVENT_QUIT) {
//                return SDL_APP_SUCCESS;
//        }
//        if (event->type == SDL_EVENT_KEY_DOWN) {
//                if (event->key.key == SDLK_ESCAPE) {
//                        return SDL_APP_SUCCESS;
//                }
//        }
//        return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate)
//{
//        float time = (SDL_GetTicks() - startTick) / 1000.0f;
//
//        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
//        SDL_RenderClear(renderer);
//
//        FaceDepth depths[6];
//        int i;
//
//        for (i = 0; i < 6; i++) {
//                float avg = 0;
//                int* idx = faceList[i];
//                for (int j = 0; j < 4; j++) {
//                        float rz;
//                        Project(cubeRaw[idx[j]], time, &rz);
//                        avg += rz;
//                }
//                depths[i].z = avg / 4.0f;
//                depths[i].face = i;
//        }
//
//        qsort(depths, 6, sizeof(FaceDepth), SortDepth);
//
//        for (i = 0; i < 6; i++) {
//                int f = depths[i].face;
//                int* idx = faceList[f];
//                SDL_Vertex v[4];
//                float z;
//
//                for (int j = 0; j < 4; j++) {
//                        Vec3 p = Project(cubeRaw[idx[j]], time, &z);
//                        v[j].position.x = p.x;
//                        v[j].position.y = p.y;
//                        v[j].color.r = (Uint8)(colors[f][0] * 255);
//                        v[j].color.g = (Uint8)(colors[f][1] * 255);
//                        v[j].color.b = (Uint8)(colors[f][2] * 255);
//                        v[j].color.a = 255;
//                        v[j].tex_coord.x = 0;
//                        v[j].tex_coord.y = 0;
//                }
//
//                int triIdx[6] = { 0,1,2, 0,2,3 };
//                SDL_RenderGeometry(renderer, NULL, v, 4, triIdx, 6);
//        }
//
//        SDL_RenderPresent(renderer);
//        return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result)
//{
//        SDL_DestroyRenderer(renderer);
//        SDL_DestroyWindow(window);
//        SDL_Quit();
//}