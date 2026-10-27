#ifndef PLAYER_STATUS_HUD_PASS_H
#define PLAYER_STATUS_HUD_PASS_H

#include <joy/joy.h>
#include "../asset_manager.h"
#include "../components/player_component.h"
#include "../components/transform_component.h"
#include "../components/action_state_component.h"
#include "../components/health_component.h"

static void player_status_hud_pass(entt::registry& ecs, uint32_t local_conv,
        jr2d_p r2d, font_p font)
{
        const char* st = "--";
        const char* dirstr = "-";
        float px = 0.0f, py = 0.0f;
        float hp = -1.0f, max_hp = (float)PLAYER_MAX_HP;
        int respawn = 0;
        bool is_dead = false;
        bool found = false;

        // 取实际逻辑窗口尺寸：HUD 位置按\"窗口百分比\"换算像素，不同分辨率都不越界/错位。
        int scr_w = 480, scr_h = 320;
        r2d_get_logical_presentation(r2d, &scr_w, &scr_h, NULL);

        ecs.view<PlayerComponent, TransformComponent, ActionStateComponent>().each(
                [&](auto e, PlayerComponent& pc, TransformComponent& t, ActionStateComponent& ac)
                {
                        if ((uint32_t)pc.conv != local_conv) return;
                        px = t.position.x;
                        py = t.position.y;
                        if (ecs.all_of<HealthComponent>(e)) {
                                auto& hc = ecs.get<HealthComponent>(e);
                                hp = (float)hc.hp;
                                max_hp = (float)hc.max_hp;
                                is_dead = hc.dead;
                                respawn = hc.respawn_timer;
                        }
                        found = true;
                        switch (ac.state) {
                        case ActionStateComponent::IDLE:   st = "IDLE";   break;
                        case ActionStateComponent::WALK:   st = "WALK";   break;
                        case ActionStateComponent::ATTACK: st = "ATTACK"; break;
                        default:                           st = "?";      break;
                        }
                        switch (ac.dir) {
                        case 0: dirstr = "D"; break;
                        case 1: dirstr = "L"; break;
                        case 2: dirstr = "R"; break;
                        case 3: dirstr = "U"; break;
                        default: dirstr = "-"; break;
                        }
                });

        char buf[128];
        if (found) {
                if (is_dead)
                        snprintf(buf, sizeof(buf), "HP: 0/%.0f  POS:(%.1f, %.1f)  ST:DEAD 重生%.1fs",
                                max_hp, px, py, respawn > 0 ? respawn / 20.0f : 0.0f);
                else if (hp >= 0.0f)
                        snprintf(buf, sizeof(buf), "HP: %.0f/%.0f  POS:(%.1f, %.1f)  ST:%s %s",
                                hp, max_hp, px, py, st, dirstr);
                else
                        snprintf(buf, sizeof(buf), "HP: --  POS:(%.1f, %.1f)  ST:%s %s",
                                px, py, st, dirstr);
        }
        else {
                snprintf(buf, sizeof(buf), "HP: --  POS:(--, --)  ST: --");
        }

        // ---- 玩家血量：底图 Health Bar.png（自带 3 个心形空槽）+ 3 个 Heart.png 填充 ----
        // 底图作血槽框，亮心按 hp/max_hp 逐颗裁剪填入 3 个槽（支持半心）；满血 100 → 3 颗全亮。
        // 位置以\"窗口百分比\"表达：左上 2%/3%，任何分辨率都在窗口内。纯渲染、不进快照。
        float bx = scr_w * 0.02f;
        float by = scr_h * 0.03f;
        float gfxH = 28.0f;   // 图形区域高度（文字基线用）
        if (found && hp >= 0.0f) {
                auto* am = AssetManager::GetInstance();
                float ratio = (max_hp > 0.0f) ? (hp / max_hp) : 0.0f;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                const int N = 3;   // 满血 3 颗

                // 底框：Health Bar.png（含 3 个心形空槽）
                const char* hb_path = am ? am->GetResourcePath(AssetManager::kCatUI, "health_bar") : nullptr;
                if (!hb_path) hb_path = "joy_side_template/sprites/objects/11-Health Bar/Health Bar.png";
                image_p hb = am ? am->GetImage(r2d, hb_path) : NULL;
                float baseW = 0.0f, baseH = gfxH;
                if (hb) {
                        image_set_blend_mode(hb, JR2D_BLEND_BLEND);
                        float hiw = 0, hih = 0;
                        image_get_size(hb, &hiw, &hih);
                        // 底框高：窗口高的 9%（限 34~90px），宽按原图比例（154:62）。
                        baseH = scr_h * 0.09f;
                        if (baseH < 34.0f) baseH = 34.0f;
                        if (baseH > 90.0f) baseH = 90.0f;
                        baseW = (hih > 0.0f) ? (baseH * hiw / hih) : baseH * 2.5f;
                        image_set_alpha(hb, 255);
                        vec2_t origin = { 0.0f, 0.0f };
                        vec2_t scl = { baseW / hiw, baseH / hih };
                        jrect_t sr = { 0.0f, 0.0f, hiw, hih };
                        image_draw_ex(hb, &sr, { bx, by }, 0.0f, scl, origin);
                }
                gfxH = baseH;

                // 填充：3 个 Heart.png 落在底框 3 个槽位（暗淡作底、亮心裁剪填充）
                const char* heart_path = "joy_side_template/sprites/objects/11-Health Bar/Heart.png";
                image_p heart = am ? am->GetImage(r2d, heart_path) : NULL;
                if (heart && baseW > 0.0f) {
                        image_set_blend_mode(heart, JR2D_BLEND_BLEND);
                        float iw = 0, ih = 0;
                        image_get_size(heart, &iw, &ih);
                        // 亮心尺寸与底框中心形对齐：宽 = 底框宽 * (22/154)，高按原图比例。
                        float heartW = baseW * (22.0f / 154.0f);
                        float heartH = (ih > 0.0f) ? (heartW * ih / iw) : heartW;
                        float y = by + (baseH - heartH) * 0.5f;   // 垂直居中于槽
                        vec2_t origin = { 0.0f, 0.0f };
                        for (int i = 0; i < N; ++i) {
                                // 每颗水平居中于底框第 i 个均分列
                                float cx = bx + baseW * ((float)i + 0.5f) / (float)N;
                                float x = cx - heartW * 0.5f;
                                float fill = ratio * (float)N - (float)i;
                                if (fill > 1.0f) fill = 1.0f;
                                // 暗淡底心
                                image_set_alpha(heart, 70);
                                vec2_t scl = { heartW / iw, heartH / ih };
                                jrect_t sr_full = { 0.0f, 0.0f, iw, ih };
                                image_draw_ex(heart, &sr_full, { x, y }, 0.0f, scl, origin);
                                // 亮心（裁左段，支持半心）
                                if (fill > 0.001f) {
                                        image_set_alpha(heart, 255);
                                        float w = heartW * fill;
                                        jrect_t sr_fill = { 0.0f, 0.0f, iw * fill, ih };
                                        vec2_t sclf = { w / iw, heartH / ih };
                                        image_draw_ex(heart, &sr_fill, { x, y }, 0.0f, sclf, origin);
                                }
                        }
                        image_set_alpha(heart, 255);
                }
        }

        // 调试文字（血量图形下方）
        jcolor_t c = { 40, 40, 45, 235 };
        r2d_set_color(r2d, c.r, c.g, c.b, c.a);
        jtext_printx(r2d, font, buf, (int)strlen(buf), c, bx, by + gfxH + scr_h * 0.015f);
}

#endif