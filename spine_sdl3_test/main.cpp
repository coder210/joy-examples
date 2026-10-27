/*
 * spine-c + SDL3 渲染测试（joy 的 jspine 模块 + ECS 用法演示）
 *
 * - 渲染/资源：joy 模块 jspine（joy/jspine.c），基于 Spine 官方 spine-sdl 封装移植到 SDL3，
 *   通过 joy 的 jr2d 拿底层 SDL_Renderer 绘制。
 * - ECS：jspine_instance_t 是纯数据结构，下面直接用 entt 把它当组件
 *   （registry.emplace<jspine_instance_t>(e)），再配一个用户侧组件做动画切换。
 *
 * 用法：
 *   spine_sdl3_test [--auto] [atlas] [skeleton] [实体数量]
 *
 * 操作：
 *   空格        所有实体切换下一个动画
 *   方向键      整体移动
 *   A / D       缩小 / 放大
 *   W / S       加速 / 减速
 *   B           显示/隐藏骨骼线
 *   P           暂停/继续
 *   F           采样一次屏幕像素
 */
#include <SDL3/SDL.h>
#include <joy/joy.h>
#include <joy/jspine.hpp>   /* RAII 包装：组件生命周期交给 ECS 托管 */
#include <entt.hpp>

#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 640

/* 用户侧组件：与 jspine_instance_t 组合使用，演示多组件协同 */
struct SpineAnimState {
        int index = 0;          /* 当前动画在资源列表中的下标 */
        float switch_timer = 0; /* 自动切换计时 */
};

static void sample_screen_pixels(SDL_Renderer *renderer) {
        SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
        int x, y, total = 0, painted = 0;
        Uint8 r, g, b, a;

        if (!surface) {
                printf("[check] SDL_RenderReadPixels failed: %s\n", SDL_GetError());
                return;
        }
        for (y = 0; y < surface->h; y += 4) {
                for (x = 0; x < surface->w; x += 4) {
                        total++;
                        if (SDL_ReadSurfacePixel(surface, x, y, &r, &g, &b, &a) &&
                            (abs((int) r - 94) > 12 || abs((int) g - 93) > 12 || abs((int) b - 96) > 12)) {
                                painted++;
                        }
                }
        }
        SDL_DestroySurface(surface);
        printf("[check] painted pixels: %d / %d (%.2f%%)\n", painted, total,
               total ? (100.0f * painted / (float) total) : 0.0f);
}

static void draw_bone_lines(SDL_Renderer *renderer, jspine_instance_p inst) {
        spSkeleton *skeleton = inst->skeleton;
        int i;
        SDL_SetRenderDrawColor(renderer, 255, 70, 70, 255);
        for (i = 0; i < skeleton->bonesCount; i++) {
                spBone *bone = skeleton->bones[i];
                float len = bone->data->length * skeleton->scaleX;
                float x1 = bone->worldX, y1 = bone->worldY;
                if (len <= 0.001f) continue;
                SDL_RenderLine(renderer, x1, y1, x1 + bone->a * len, y1 + bone->c * len);
        }
}

int main(int argc, char **argv) {
        const char *atlas_path = "spineboy.atlas";
        const char *skeleton_path = "spineboy-pro.json";
        int auto_mode = 0;
        int entity_count = 3;
        int positional = 0;
        int number_arg = 0;
        int i;

        SDL_Window *window = NULL;
        SDL_Renderer *renderer = NULL;
        jr2d_p r2d = NULL;
        jspine::asset asset;                 /* RAII：析构自动 jspine_asset_free */
        entt::registry registry;
        int quit = 0;
        int draw_bones = 0;
        int paused = 0;
        float time_scale = 1.0f;
        float scale_mul = 1.0f;
        float offset_x = 0, offset_y = 0;
        Uint64 last_ticks, fps_ticks, frame_count = 0;
        Uint64 fps_frames = 0;

        setvbuf(stdout, NULL, _IONBF, 0);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "--auto") == 0) { auto_mode = 1; continue; }
                if (positional == 0) { atlas_path = argv[i]; positional++; continue; }
                if (positional == 1) { skeleton_path = argv[i]; positional++; continue; }
                if (number_arg == 0) { entity_count = atoi(argv[i]); number_arg++; continue; }
        }
        if (entity_count < 1) entity_count = 1;
        if (entity_count > 64) entity_count = 64;

        if (!SDL_Init(SDL_INIT_VIDEO)) {
                printf("SDL_Init failed: %s\n", SDL_GetError());
                return 1;
        }
        window = SDL_CreateWindow("joy jspine + ECS (entt)", WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_RESIZABLE);
        if (!window) { printf("SDL_CreateWindow failed: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
        renderer = SDL_CreateRenderer(window, NULL);
        if (!renderer) {
                printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
        }
        r2d = r2d_wrap(renderer); /* 用已有 SDL_Renderer 包装出 joy 的 r2d */
        printf("[sdl] renderer: %s\n", SDL_GetRendererName(renderer));

        /* ---- 共享资源（RAII：离开作用域/重置时自动释放） ---- */
        asset = jspine::asset(r2d, atlas_path, skeleton_path, 0.5f);
        if (!asset.valid()) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
        }

        printf("---- joy jspine self check ----\n");
        printf("animations     : %d\n", asset.animation_count());
        for (i = 0; i < asset.animation_count() && i < 12; i++) {
                printf("  %s (%.2fs)\n", asset.animation_name(i), asset.animation_duration(i));
        }
        printf("skeleton data  : bones=%d slots=%d\n",
               jspine_asset_data(asset.get())->bonesCount,
               jspine_asset_data(asset.get())->slotsCount);
        printf("--------------------------------\n");

        /* ---- ECS：emplace 即 init，destroy 即 free（无需手工管理） ---- */
        for (i = 0; i < entity_count; i++) {
                auto e = registry.create();
                auto &spine = registry.emplace<jspine::instance>(e, asset); /* 构造时 init */
                auto &anim = registry.emplace<SpineAnimState>(e);

                if (!spine.valid()) {
                        printf("[ecs] bind failed\n");
                        registry.destroy(e);   /* 析构自动 free */
                        continue;
                }
                const char *run = asset.animation_name(0);
                for (int a = 0; a < asset.animation_count(); a++) {
                        if (strcmp(asset.animation_name(a), "run") == 0) {
                                run = asset.animation_name(a);
                                anim.index = a;
                                break;
                        }
                }
                spine.set_animation(0, run, true);
                spine.set_position(160.0f + i * 220.0f, WINDOW_HEIGHT - 60.0f);
                spine.set_scale(1.0f, 1.0f);
                spine.data().time_scale = 0.8f + i * 0.2f;  /* 每个实体不同速度 */
                spine.data().zorder = i;
                if (i == 1) spine.set_flip_x(true);         /* 演示翻转 */
                if (i == 2) {
                        jcolor_t tint = {255, 180, 180, 255};
                        spine.set_tint(tint);               /* 演示染色 */
                        spine.set_alpha(0.85f);             /* 演示透明度 */
                }
        }
        printf("[ecs] entities: %d (RAII 组件，销毁实体自动释放)\n",
               (int) registry.view<jspine::instance>().size());
        printf("[help] space=next anim  K=kill one  arrows=move  A/D=scale  W/S=speed  B=bones  P=pause  F=pixel check\n");

        last_ticks = SDL_GetTicksNS();
        fps_ticks = last_ticks;

        while (!quit) {
                SDL_Event e;
                Uint64 now;
                float dt;

                while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_EVENT_QUIT) quit = 1;
                        else if (e.type == SDL_EVENT_KEY_DOWN) {
                                SDL_Keycode key = e.key.key;
                                if (key == SDLK_ESCAPE) quit = 1;
                                else if (key == SDLK_SPACE) {
                                        /* system：同时访问两个组件 */
                                        registry.view<jspine::instance, SpineAnimState>().each(
                                                [&](jspine::instance &spine, SpineAnimState &anim) {
                                                        int count = asset.animation_count();
                                                        if (count <= 0) return;
                                                        anim.index = (anim.index + 1) % count;
                                                        spine.set_animation(0, asset.animation_name(anim.index), true);
                                                });
                                        printf("[ecs] all entities -> next animation\n");
                                } else if (key == SDLK_K) {
                                        /* 销毁实体：组件析构自动 jspine_instance_free，无需手工释放 */
                                        auto v = registry.view<jspine::instance>();
                                        if (!v.empty()) {
                                                registry.destroy(v.back());
                                                printf("[ecs] destroyed 1 entity, remaining=%d\n",
                                                       (int) registry.view<jspine::instance>().size());
                                        }
                                } else if (key == SDLK_B) draw_bones = !draw_bones;
                                else if (key == SDLK_P) paused = !paused;
                                else if (key == SDLK_A) scale_mul = std::max(0.2f, scale_mul - 0.1f);
                                else if (key == SDLK_D) scale_mul = std::min(3.0f, scale_mul + 0.1f);
                                else if (key == SDLK_W) time_scale = std::min(4.0f, time_scale + 0.25f);
                                else if (key == SDLK_S) time_scale = std::max(0.0f, time_scale - 0.25f);
                                else if (key == SDLK_F) sample_screen_pixels(renderer);
                        }
                }

                now = SDL_GetTicksNS();
                dt = (float) ((double) (now - last_ticks) / 1000000000.0);
                last_ticks = now;
                if (dt > 0.1f) dt = 0.1f;

                {
                        const bool *keys = SDL_GetKeyboardState(NULL);
                        float speed = 260.0f * dt;
                        if (keys[SDL_SCANCODE_LEFT]) offset_x -= speed;
                        if (keys[SDL_SCANCODE_RIGHT]) offset_x += speed;
                        if (keys[SDL_SCANCODE_UP]) offset_y -= speed;
                        if (keys[SDL_SCANCODE_DOWN]) offset_y += speed;
                }

                /* ---- system 1：先写组件字段，再推进动画（顺序反了会滞后一帧） ---- */
                registry.view<jspine::instance>().each([&](jspine::instance &spine) {
                        spine.data().x += offset_x;
                        spine.data().y += offset_y;
                        spine.set_scale(scale_mul, scale_mul);
                        if (!paused) spine.update(dt * time_scale * spine.data().time_scale);
                        else spine.sync(); /* 暂停时也要同步变换 */
                });
                offset_x = offset_y = 0;

                SDL_SetRenderDrawColor(renderer, 94, 93, 96, 255);
                SDL_RenderClear(renderer);

                /* ---- system 2：按 zorder 排序后绘制 ---- */
                {
                        std::vector<jspine::instance *> batch;
                        registry.view<jspine::instance>().each([&](jspine::instance &spine) {
                                if (!spine.data().visible) return;
                                batch.push_back(&spine);
                        });
                        std::sort(batch.begin(), batch.end(), [](jspine::instance *a, jspine::instance *b) {
                                return a->data().zorder < b->data().zorder;
                        });
                        for (jspine::instance *spine : batch) {
                                spine->draw(r2d);
                                if (draw_bones) draw_bone_lines(renderer, spine->ptr());
                        }
                }

                frame_count++;
                if (auto_mode && frame_count == 60) {
                        /* 自动验证：销毁一半实体，组件析构应自动释放且不崩溃 */
                        int before = (int) registry.view<jspine::instance>().size();
                        for (int k = 0; k < before / 2; k++) {
                                auto v = registry.view<jspine::instance>();
                                if (v.empty()) break;
                                registry.destroy(v.back());
                        }
                        printf("[auto] destroyed %d entities: %d -> %d\n", before / 2, before,
                               (int) registry.view<jspine::instance>().size());
                }
                if (auto_mode && frame_count >= 120) {
                        printf("[auto] rendered %llu frames, run pixel check...\n",
                               (unsigned long long) frame_count);
                        sample_screen_pixels(renderer);
                        break;
                }

                SDL_RenderPresent(renderer);

                fps_frames++;
                if (now - fps_ticks >= 500000000ULL) {
                        float fps = (float) ((double) fps_frames / ((double) (now - fps_ticks) / 1000000000.0));
                        auto view = registry.view<jspine::instance>();
                        printf("[frame] fps=%.1f entities=%d anim=%s\n", fps, (int) view.size(),
                               view.empty() ? "(none)"
                                            : jspine_instance_animation(view.get<jspine::instance>(view.front()).ptr()));
                        fps_frames = 0;
                        fps_ticks = now;
                }
        }

        /* ---- 清理：没有任何手工 free，全部由 RAII/ECS 托管 ---- */
        registry.clear();          /* 组件析构 -> jspine_instance_free（可省略，registry 析构也会做） */
        asset.reset();             /* 释放共享资源（可省略，离开作用域析构也会做） */
        r2d_destroy(r2d);          /* 内部会 SDL_DestroyRenderer，不要再单独销毁 renderer */
        SDL_DestroyWindow(window);
        SDL_Quit();
        printf("[jspine] test finished.\n");
        return 0;
}
