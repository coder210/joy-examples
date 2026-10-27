#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "gameplay_controls_layer.h"
#include "../ui_style.h"
#include "../asset_manager.h"
#include "../components/player_component.h"
#include "../components/action_state_component.h"

static const float GC_PI = 3.14159265f;

// 本地玩家某项冷却剩余比例（0..1 = 剩余/总量）。无本地玩家或未冷却返回 0。
// coolType: 1=攻(炸弹)。（原"射"枪械/"线"射线已移除，无对应冷却）
float GameplayControlsLayer::localCoolRatio(int coolType)
{
        if (!ctx_ || net_session(ctx_).local_conv == 0) return 0.0f;
        auto view = ctx_->ecs.view<PlayerComponent, ActionStateComponent>();
        for (auto e : view) {
                auto& pc  = view.get<PlayerComponent>(e);
                if (pc.conv != (int)net_session(ctx_).local_conv) continue;
                auto& act = view.get<ActionStateComponent>(e);
                int cd = 0, max = 1;
                if (coolType == 1)      { cd = act.melee_cooldown;   max = ActionStateComponent::MELEE_COOLDOWN; }
                else return 0.0f;
                if (cd <= 0) return 0.0f;
                return (float)cd / (float)max;
        }
        return 0.0f;
}

// 墨色环形冷却进度：以 (cx,cy) 圆心、半径 R 的圆环（厚 thick），从正上方顺时针填充
// 已耗进度 frac∈[0,1]。纯表现层，不影响帧同步。
void GameplayControlsLayer::drawCoolRing(float cx, float cy,
                float R, float thick, float frac)
{
        if (!ctx_ || frac <= 0.01f) return;
        jr2d_p r2d = ctx_->r2d;
        constexpr int SEGS = 40;
        if (frac > 1.0f) frac = 1.0f;
        int filled = (int)(frac * SEGS);
        if (filled < 7) filled = 7;                 // 保证可见的一小段弧
        if (filled > SEGS) filled = SEGS;
        const float a0 = -GC_PI * 0.5f;
        r2d_set_color(r2d, 40, 40, 45, 200);        // 深墨
        for (int i = 0; i < filled; i++) {
                float a1 = a0 + (float)i * (2.0f * GC_PI / SEGS);
                float a2 = a0 + (float)(i + 1) * (2.0f * GC_PI / SEGS);
                for (int k = 0; k < (int)thick; k++) {
                        float rr = R + (float)k;
                        shape_draw_line(r2d, cx + cosf(a1) * rr, cy + sinf(a1) * rr,
                                              cx + cosf(a2) * rr, cy + sinf(a2) * rr);
                }
        }
}

// 圆按钮右下角叠加键盘键角标（方案 A，对齐 3D）：右下 1/4 处深墨小字母。
void GameplayControlsLayer::drawKeyBadge(font_p font, const char* key,
                float bx, float by)
{
        if (!ctx_) return;
        jr2d_p r2d = ctx_->r2d;
        if (!r2d || !font || !key || !key[0]) return;
        float size = 14.0f;                          // 角标像素高度（与 font 实际字号一致）
        float tw = size * 0.5f;                      // 单字符宽≈fontSize/2
        jcolor_t c = { 20, 20, 24, 235 };
        float px = floorf(bx - tw * 0.5f + 0.5f);    // 对齐到最近整数像素
        float py = floorf(by - size * 0.5f + 0.5f);
        jtext_printx(r2d, font, key, (int)strlen(key), c, px, py);
}

// 单个圆形按钮绘制（去掉 node 依赖后由 Render 直接调用）
void GameplayControlsLayer::drawButton(const gc_button_meta* m)
{
        if (!ctx_ || !m || !m->btn) return;
        jr2d_p r2d = ctx_->r2d;
        float cx = m->x + m->w * 0.5f;
        float cy = m->y + m->h * 0.5f;
        float R  = (m->w < m->h ? m->w : m->h) * 0.5f;

        // 方形按钮：不透明宣纸色底（原底色是暗墨，与黑字同色系导致文字看不清，
        // 改为浅底 + 黑字形成高对比）+ 暗墨方边框。
        bool pressed = (m->btn == pressed_btn);   // 当前按住中的按钮（方向/技能）
        jrect_t rect = { m->x, m->y, m->w, m->h };
        if (pressed) r2d_set_color(r2d, 222, 220, 213, 255);   // 按下：略压深作反馈
        else         r2d_set_color(r2d, 248, 246, 240, 255);   // 常态：宣纸色（不透明）
        shape_draw_rectangle(r2d, "fill", rect);
        r2d_set_color(r2d, 34, 34, 38, 220);                   // 方形暗墨边框
        shape_draw_rectangle(r2d, "line", rect);
        float out = R + 2.0f;

        // 按钮文字：黑色（浅底上清晰可辨）
        if (m->label && m->label[0]) {
                font_p f = AssetManager::GetInstance()->GetSimheiFont24();
                jcolor_t tc = { 20, 20, 24, 255 };
                jtext_printx(r2d, f, m->label, (int)strlen(m->label), tc,
                        cx - 12.0f, cy - 12.0f);
        }

        // 冷却环：比最外描边略大一圈（深墨）
        if (m->coolType != 0) {
                float ratio = localCoolRatio(m->coolType);
                if (ratio > 0.0f)
                        drawCoolRing(cx, cy, out + 1.0f, 4.0f, 1.0f - ratio);
        }

        // 键盘角标：右下 1/4 处
        if (m->keyLabel && m->keyLabel[0]) {
                float off = R * 0.68f;
                float bx = cx + off * 0.7071f;
                float by = cy + off * 0.7071f;
                font_p f = AssetManager::GetInstance()->GetSimheiFont12();
                drawKeyBadge(f, m->keyLabel, bx, by);
        }
}

void GameplayControlsLayer::sendGameplayControlEvent(int event_code)
{
        push_user_event(GAMEPLAY_CONTROL_EVENT, event_code);
}

GameplayControlsLayer::GameplayControlsLayer(Context* ctx) : UILayer(ctx)
{
        load_dpad_panel();
}

GameplayControlsLayer::~GameplayControlsLayer() = default;

void GameplayControlsLayer::add_button(int idx, button_p* slot,
                float cx, float cy, float wf, float hf,
                const char* label, const char* keyLabel, int coolType)
{
        button_p b = button_create(ctx_->r2d, cx, cy, wf, hf);
        UIStyle::applyInkButton(b);
        *slot = b;
        int lw = 0, lh = 0;
        r2d_get_logical_presentation(ctx_->r2d, &lw, &lh, NULL);
        float w = wf * (float)lw, h = hf * (float)lh;
        float x = (cx - wf * 0.5f) * (float)lw, y = (cy - hf * 0.5f) * (float)lh;
        metas[idx] = { b, ctx_, w, h, x, y, label, keyLabel, coolType };
}

void GameplayControlsLayer::load_dpad_panel()
{
        // 操作：左下角 WASD 倒 T 组合（左手拇指）——W(跳) 居中上方，A(左)/S(下)/D(右) 同排；
        // 攻(J) 单独放右侧（右手拇指）。S 键恢复占位，发 GAMEPLAY_CONTROL_DOWN（功能后续接入）。
        // 尺寸收至 80%（0.100×0.155 → 0.080×0.124），行列间距同步收紧，保持紧凑不重叠。
        add_button(2, &btn_jump,  0.19000f, 0.72600f, 0.08000f, 0.12400f, "跳", "W", 0);
        add_button(0, &btn_left,  0.10000f, 0.86000f, 0.08000f, 0.12400f, "左", "A", 0);
        add_button(3, &btn_down,  0.19000f, 0.86000f, 0.08000f, 0.12400f, "下", "S", 0);
        add_button(1, &btn_right, 0.28000f, 0.86000f, 0.08000f, 0.12400f, "右", "D", 0);
        // 攻：右下角右手拇指位，高度与左下 WASD 排大致齐平，离右边缘留约 7% 余量。
        add_button(4, &btn_attack,0.88000f, 0.83000f, 0.10000f, 0.15000f, "攻", "J", 1);
        // 注：原"射"(枪械子弹)与"线"(射线大招)两个按钮及其功能已移除，玩家仅保留"攻"(投掷炸弹)。
}

void GameplayControlsLayer::HandleEvent(const void* ev)
{
        jevent_callbacks_t cbs = {};
        cbs.on_mousepressed = [](float x, float y, int btn, void* arg) {
                (void)btn;
                auto* self = (GameplayControlsLayer*)arg;
                auto hit = [&](button_p b) { return button_is_pressed(b, x, y); };
                if (self->pressed_btn) return;   // 已按住一个钮，忽略后续按下
                if (self->btn_left && hit(self->btn_left)) {
                        self->pressed_btn = self->btn_left; self->pressed_code = GAMEPLAY_CONTROL_RELEASE_LEFT;
                        GameplayControlsLayer::sendGameplayControlEvent(GAMEPLAY_CONTROL_LEFT);
                } else if (self->btn_right && hit(self->btn_right)) {
                        self->pressed_btn = self->btn_right; self->pressed_code = GAMEPLAY_CONTROL_RELEASE_RIGHT;
                        GameplayControlsLayer::sendGameplayControlEvent(GAMEPLAY_CONTROL_RIGHT);
                } else if (self->btn_jump && hit(self->btn_jump)) {
                        self->pressed_btn = self->btn_jump; self->pressed_code = GAMEPLAY_CONTROL_RELEASE_UP;
                        GameplayControlsLayer::sendGameplayControlEvent(GAMEPLAY_CONTROL_UP);
                } else if (self->btn_down && hit(self->btn_down)) {
                        self->pressed_btn = self->btn_down; self->pressed_code = GAMEPLAY_CONTROL_RELEASE_DOWN;
                        GameplayControlsLayer::sendGameplayControlEvent(GAMEPLAY_CONTROL_DOWN);
                } else if (self->btn_attack && hit(self->btn_attack) && self->localCoolRatio(1) <= 0.0f) {
                        self->pressed_btn = self->btn_attack; self->pressed_code = GAMEPLAY_CONTROL_ATTACK;
                }
        };
        cbs.on_mousereleased = [](float x, float y, int btn, void* arg) {
                (void)btn;
                auto* self = (GameplayControlsLayer*)arg;
                if (!self->pressed_btn) return;
                button_p b = self->pressed_btn;
                int code = self->pressed_code;
                self->pressed_btn = NULL;
                self->pressed_code = 0;
                button_set_pressed(b, false);
                if (b == self->btn_attack) {
                        // 技能：完整点击（松开仍在按钮内）才触发一次
                        if (!button_is_hovered(b, x, y)) return;
                        if (self->localCoolRatio(1) > 0.0f) return;
                        GameplayControlsLayer::sendGameplayControlEvent(code);
                } else {
                        // 方向键：松开即清除对应位移位
                        GameplayControlsLayer::sendGameplayControlEvent(code);
                }
        };
        dispatch_event(cbs, ev, this);
}

void GameplayControlsLayer::Destroy()
{
        for (int i = 0; i < 8; i++) {
                if (metas[i].btn) { button_destroy(metas[i].btn); metas[i].btn = NULL; }
        }
        btn_left = btn_right = btn_jump = btn_down = NULL;
        btn_attack = NULL;
        pressed_btn = NULL;
        pressed_code = 0;
}

void GameplayControlsLayer::Render()
{
        if (!ctx_) return;
        for (int i = 0; i < 8; i++) drawButton(&metas[i]);
}
