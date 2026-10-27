#ifndef DRAW_PLAYER_PASS_H
#define DRAW_PLAYER_PASS_H
#include <joy/joy.h>
#include <joy/janim.h>          /* joy 原生帧动画状态机：支持精灵表裁剪 + 状态过渡 */
#include <string>
#include <unordered_map>
#include <cstring>
#include "../components/global_state_component.h"
#include "../components/id_component.h"
#include "../components/player_component.h"
#include "../components/action_state_component.h"
#include "../components/health_component.h"
#include "../components/transform_component.h"
#include "../systems/character_catalog.h"
#include "../components/anim_state_component.h"
#include "../components/logic_velocity_component.h"
#include "../asset_manager.h"
#include "../protocol.h"
#include <vector>
#include <cstdlib>
#include <cmath>

struct Context;

// ============================================================
// 玩家脚部粒子（跑/跳/落尘土）—— 纯渲染，挂 PlayerParticleComponent 于玩家实体。
// 素材为 6 张独立 PNG（joy_side_template/sprites/objects/13~15-* Particles/1..6.png），
// 按寿命推进帧、按 alpha 淡出；不影响帧同步、不进快照。
// ============================================================
#ifndef PLAYER_PARTICLE_FRAMES
#define PLAYER_PARTICLE_FRAMES 6
#endif

struct PParticle {
    float wx = 0, wy = 0;          // 世界坐标（米）
    float vx = 0, vy = 0;          // 世界速度（米/秒，+y 向下）
    float life = 0, max_life = 1;
    float size = 0.5f;             // 目标高（米），按种类调，避免跳跃尘过大像白气
    int   frame = 0;
    int   type = 0;                // 0=跑 1=跳 2=落
    bool  flip = false;
};

struct PlayerParticleComponent {
    float prev_vy = 0.0f;          // 上一帧垂直速度（检测起跳瞬间用）
    // 跑动尘按"每秒个数"累积发射（与渲染帧率无关）：累加器存不足 1 个的余量。
    float run_emit_acc = 0.0f;
    std::vector<PParticle> parts;
};

static std::unordered_map<std::string, image_p> g_particle_img_cache;

static image_p get_particle_image(Context* ctx, int type, int frame)
{
    // 目录走 AssetManager 资源索引（resources.json 的 particles/run|jump|fall），
    // 缺失时回退内置默认目录，不在此硬编码对象文件夹路径。
    static const char* kPartIds[3] = { "run", "jump", "fall" };
    static const char* kDefDirs[3] = {
        "joy_side_template/sprites/objects/13-Run Particles",
        "joy_side_template/sprites/objects/14-Jump Particles",
        "joy_side_template/sprites/objects/15-Fall Particles"
    };
    if (type < 0 || type > 2) type = 0;
    if (frame < 0) frame = 0;
    if (frame >= PLAYER_PARTICLE_FRAMES) frame = PLAYER_PARTICLE_FRAMES - 1;
    const char* dir = NULL;
    auto* am = AssetManager::GetInstance();
    if (am) dir = am->GetResourcePath(AssetManager::kCatParticles, kPartIds[type]);
    if (!dir) dir = kDefDirs[type];
    char buf[160];
    snprintf(buf, sizeof(buf), "%s/%d.png", dir, frame + 1);
    std::string key(buf);
    auto it = g_particle_img_cache.find(key);
    if (it != g_particle_img_cache.end()) return it->second;
    image_p img = (am && ctx) ? am->GetImage(ctx->r2d, key.c_str()) : NULL;
    if (img) image_set_blend_mode(img, JR2D_BLEND_BLEND);   // 启用透明混合（一次性）
    return g_particle_img_cache[key] = img;
}

// 在脚下 (wx,wy) 发射一个粒子；back = 朝向后方（+1 右 / -1 左 / 0 无），跑动尘向后扬。
static void player_emit_particle(PlayerParticleComponent& ppc, float wx, float wy,
                                 int type, float back)
{
    if (ppc.parts.size() >= 80) return;
    PParticle p;
    p.wx = wx + ((float)rand() / RAND_MAX - 0.5f) * 0.18f;
    p.wy = wy;
    p.type = type;
    p.flip = ((float)rand() / RAND_MAX < 0.5f);
    if (type == 1) {                 // 起跳：脚下小迸发，偏小且贴地，避免两侧像白气
        float ang = (float)rand() / RAND_MAX * 3.1415926f * 2.0f;
        float spd = 0.4f + (float)rand() / RAND_MAX * 0.5f;
        p.vx = cosf(ang) * spd;
        p.vy = -(0.3f + (float)rand() / RAND_MAX * 0.4f);   // 向上（-y），幅度收敛
        p.size = 0.25f;
    } else if (type == 2) {          // 落地：脚下短促小尘，快速消散
        p.vx = back * 0.4f + ((float)rand() / RAND_MAX - 0.5f) * 0.6f;
        p.vy = -(0.1f + (float)rand() / RAND_MAX * 0.2f);
        p.size = 0.25f;
    } else {                         // 跑动：向后扬尘
        p.vx = back * 0.8f + ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        p.vy = -(0.2f + (float)rand() / RAND_MAX * 0.4f);
        p.size = 0.45f;
    }
    p.max_life = (type == 1) ? 0.35f : (type == 0 ? 0.35f : 0.3f);
    p.life = p.max_life;
    p.frame = 0;
    ppc.parts.push_back(p);
}

// 推进并绘制玩家粒子（在角色精灵之下绘制，呈现脚下尘土）。每粒子绘制前设自身 alpha。
static void player_update_draw_particles(Context* ctx, GlobalState& gs,
                                         PlayerParticleComponent& ppc, float z, float dt)
{
    auto& cam = gs.camera_position;
    for (size_t i = 0; i < ppc.parts.size(); ) {
        PParticle& p = ppc.parts[i];
        p.life -= dt;
        if (p.life <= 0.0f) { ppc.parts.erase(ppc.parts.begin() + i); continue; }
        p.wx += p.vx * dt;
        p.wy += p.vy * dt;
        p.vy += 1.5f * dt;             // 轻微重力（尘先上飘后回落）
        float tt = 1.0f - p.life / p.max_life;
        p.frame = (int)(tt * PLAYER_PARTICLE_FRAMES);
        if (p.frame < 0) p.frame = 0;
        if (p.frame >= PLAYER_PARTICLE_FRAMES) p.frame = PLAYER_PARTICLE_FRAMES - 1;

        image_p img = get_particle_image(ctx, p.type, p.frame);
        if (img) {
            float sx = meters_to_pixels(p.wx - cam.x) * z;
            float sy = meters_to_pixels(p.wy - cam.y) * z;
            float iw = 0, ih = 0;
            image_get_size(img, &iw, &ih);
            float scale = (ih > 0.0f) ? (kEntityRenderScale * p.size * 50.0f * z / ih) : 1.0f;  // 按每粒 size 高（随角色同步放大）
            jrect_t sr = { 0, 0, iw, ih };
            vec2_t pos = { sx, sy };
            vec2_t scl = { p.flip ? -scale : scale, scale };
            vec2_t origin = { iw * 0.5f, ih * 0.5f };
            int a = (int)(255.0f * p.life / p.max_life);
            if (a < 0) a = 0; if (a > 255) a = 255;
            image_set_alpha(img, (uint8_t)a);   // 共享纹理：绘制前设自身 alpha
            image_draw_ex(img, &sr, pos, 0.0f, scl, origin);
        }
        i++;
    }
}

/* ============================================================
 * 玩家序列帧渲染（characters.json 驱动）
 *  - 动画定义/播放用 joy 的 janim（anim_def_t + anim_t + anim_update）
 *  - 素材为横向等帧精灵表，帧宽 = 图高，帧数 = 图宽 / 图高（自动探测）
 *  - 纯渲染层：只写 anim_t，绝不回写 LogicPosition/Health 等逻辑组件，
 *    不参与帧同步、不进快照。
 * ============================================================ */

 // 播放帧率（帧/秒）。素材未携带帧率，统一取 12 FPS。
static const float kSpriteFPS = 12.0f;

// 渲染层缓存（不属于帧同步状态）：
//  - 纹理缓存统一收归 AssetManager::tex_cache_（见 sprite_get_image），本文件不再持有；
//  - 每实体的动画播放状态改为 AnimStateComponent 挂在实体上（实体销毁由 ECS 自动回收，
//    消除此前裸全局 map 的泄漏）；
//  - 仅保留按角色类型共享的动画定义缓存（key=char_id），随场景销毁清理。
static std::unordered_map<int, anim_def_t>      g_anim_defs;      // key: char_id
// 按路径指针缓存已解析纹理：clip.path 是 anim_def_t 内稳定的 char[] 地址（整局不变），
// 用指针作 key 可省去每帧的字符串哈希查找（AssetManager 内部仍是 unordered_map<string>）。
static std::unordered_map<const char*, image_p> g_img_cache;      // key: clip.path

// 清空玩家动画定义缓存（纹理归 AssetManager::ClearTexCache 管理）
static void player_anim_cache_clear()
{
        g_anim_defs.clear();
        g_img_cache.clear();
}

static image_p sprite_get_image(Context* ctx, const char* path)
{
        if (!path || !*path) return NULL;
        auto it = g_img_cache.find(path);          // 指针比较，O(1) 无字符串哈希
        if (it != g_img_cache.end()) return it->second;
        // 纹理缓存统一走 AssetManager（与字体/地图资源同一入口，生命周期由销毁时统一释放）
        auto* am = AssetManager::GetInstance();
        image_p img = (am && ctx) ? am->GetImage(ctx->r2d, path) : NULL;
        return g_img_cache[path] = img;
}

// 把一整条横向精灵表展开为若干 clip（每帧一个 src_rect）
static void sprite_add_clips(Context* ctx, anim_def_t* def, int state, const std::string& path)
{
        if (path.empty()) return;
        // 构建期直接走 AssetManager（按 string 稳定缓存纹理），不进 g_img_cache：
        // 这里的 path 是 animPath() 返回的临时 std::string，其 c_str() 出作用域即失效，
        // 不能用作 g_img_cache 的指针 key（会存悬空指针）。渲染期 clip.path 来自 def 内
        // 稳定 char[]，才由 sprite_get_image 走指针缓存。
        auto* am = AssetManager::GetInstance();
        image_p img = (am && ctx) ? am->GetImage(ctx->r2d, path.c_str()) : NULL;
        if (!img) return;

        float w = 0, h = 0;
        if (!image_get_size(img, &w, &h) || h <= 0.0f) return;
        int frames = (int)(w / h);
        if (frames <= 0) return;
        if (frames > ANIM_MAX_CLIPS) frames = ANIM_MAX_CLIPS;

        for (int i = 0; i < frames; i++) {
                jrect_t r = { (float)i * h, 0.0f, h, h };
                anim_add_clip(def, state, path.c_str(), 1.0f / kSpriteFPS, r);
        }
}

// 构建（并缓存）某角色的动画状态机
static const anim_def_t& sprite_anim_def(Context* ctx, int char_id)
{
        auto it = g_anim_defs.find(char_id);
        if (it != g_anim_defs.end()) return it->second;

        const CharacterDef& cd = g_characterCatalog.get(char_id);
        anim_def_t def;
        memset(&def, 0, sizeof(def));

        // 状态下标必须与 CharacterAnimId 对齐：
        // janim 内部按 current_state ∈ [2,5] 判定"攻击态"以防止重复触发，故 attack 必须是 2。
        int s_idle = anim_add_state(&def, "idle", true);
        int s_walk = anim_add_state(&def, "walk", true);
        int s_attack = anim_add_state(&def, "attack", false);
        int s_hit = anim_add_state(&def, "hit", false);
        int s_death = anim_add_state(&def, "death", false);
        int s_knocked = anim_add_state(&def, "knocked", false);
        (void)s_hit;
        // hit 仍是"已定义但无触发源"的休眠状态（未加任何过渡），故不播放。
        // knocked 已接线：bullet_system / melee_attack_system 在玩家受伤时把
        // ActionStateComponent::knockback_timer 置满 KNOCKED_DURATION，渲染端据此触发（见 p.knocked）。

        sprite_add_clips(ctx, &def, s_idle, g_characterCatalog.animPath(cd, ANIM_IDLE));
        sprite_add_clips(ctx, &def, s_walk, g_characterCatalog.animPath(cd, ANIM_WALK));
        sprite_add_clips(ctx, &def, s_attack, g_characterCatalog.animPath(cd, ANIM_ATTACK));  // 素材缺失则无 clip
        sprite_add_clips(ctx, &def, s_hit, g_characterCatalog.animPath(cd, ANIM_HIT));
        sprite_add_clips(ctx, &def, s_death, g_characterCatalog.animPath(cd, ANIM_DEATH));
        sprite_add_clips(ctx, &def, s_knocked, g_characterCatalog.animPath(cd, ANIM_KNOCKED));

        anim_set_default_state(&def, s_idle);

        // 静止 ↔ 步行（按 speed 参数）
        int t = anim_add_transition(&def, s_idle, s_walk);
        anim_transition_add_cond(&def, t, "speed", ANIM_COND_GREATER, 0.0f);
        t = anim_add_transition(&def, s_walk, s_idle);
        anim_transition_add_cond(&def, t, "speed", ANIM_COND_EQUALS, 0.0f);
        // 任意态 → 攻击（trigger）；攻击播完 → 回到 idle
        t = anim_add_transition(&def, -1, s_attack);
        anim_transition_add_cond(&def, t, "attack", ANIM_COND_TRIGGER, 0.0f);
        t = anim_add_transition(&def, s_attack, s_idle);
        anim_transition_set_exit_time(&def, t,
                (float)ActionStateComponent::ATTACK_DURATION / 20.0f);  // 20Hz 逻辑 tick

        // 任意态 → 击退（trigger）；击退播完 → 回到 idle
        t = anim_add_transition(&def, -1, s_knocked);
        anim_transition_add_cond(&def, t, "knocked", ANIM_COND_TRIGGER, 0.0f);
        t = anim_add_transition(&def, s_knocked, s_idle);
        anim_transition_set_exit_time(&def, t,
                (float)ActionStateComponent::KNOCKED_DURATION / 20.0f);  // 20Hz 逻辑 tick

        g_anim_defs[char_id] = def;
        return g_anim_defs[char_id];
}

// 玩家序列帧：推进动画状态机 + 绘制（合并原 player_anim_pass 与 draw_player_pass，
// 单次遍历玩家实体，省去两次 view 迭代与两次动画定义查表；纯渲染、不参与帧同步、不进快照）。
// dt 为渲染帧真实步长（由 GameScene 在 Update 阶段存下，Render 阶段传入）。

static void player_render_pass(entt::registry& registry, Context* ctx, float dt)
{
        auto& gs_p = registry.ctx().get<GlobalState>();
        auto& cam = gs_p.camera_position;
        const float z = (gs_p.view_zoom > 0.0f) ? gs_p.view_zoom : 1.0f;
        auto view = registry.view<PlayerComponent, ActionStateComponent, TransformComponent>();

        for (auto e : view) {
                auto& pc = view.get<PlayerComponent>(e);
                auto& ac = view.get<ActionStateComponent>(e);
                auto& t = view.get<TransformComponent>(e);

                // 每实体动画状态挂在 AnimStateComponent 上（实体销毁自动回收，无泄漏）；
                // 首次创建时一次性缓存其角色动画定义，避免每帧按 char_id 查 g_anim_defs。
                auto& asc = registry.get_or_emplace<AnimStateComponent>(e);
                if (!asc.def) asc.def = &sprite_anim_def(ctx, pc.char_id);
                const anim_def_t& def = *asc.def;
                anim_t& a = asc.anim;

                // 垂直速度与着地判定：半空中死亡时先由重力把身体落到地面（见 movement_system），
                // 落地后再切死亡动画，避免"定格在半空播死亡动画"。
                float pv_y = 0.0f;
                if (registry.all_of<LogicVelocityComponent>(e))
                        pv_y = fp_to_float(registry.get<LogicVelocityComponent>(e).y);
                const bool grounded = (pv_y > -0.05f && pv_y < 0.05f);

                // 死亡：janim 的 params 只有 speed/dir/attack，无 death 条件，
                // 故由外部强制切到 death 状态（janim 会挡住从 death 误触发 attack）。
                bool dead = registry.all_of<HealthComponent>(e)
                        && registry.get<HealthComponent>(e).dead;
                if (dead && grounded && a.current_state != ANIM_DEATH) {   // 需落地才播死亡动画
                        a.current_state = ANIM_DEATH;
                        a.current_frame = 0;
                        a.frame_elapsed = 0;
                        a.state_elapsed = 0;
                }
                // 复活：死亡态必须由渲染端主动退出。
                // player_death_system 重生时只把 HealthComponent.dead 置回 false、
                // 并复位 ActionStateComponent 等逻辑状态；但 janim 的 anim_t.current_state
                // 是纯渲染态，没有逻辑系统会改它，且 anim_def 里也没有 death→idle 的过渡
                // （只有 idle↔walk / any→attack / attack→idle / any→knocked / knocked→idle），
                // 因此不在这里复位的话，玩家复活后动画会永远停在 death（“尸体姿势复活”）。
                if (!dead && a.current_state == ANIM_DEATH) {
                        a.current_state = ANIM_IDLE;
                        a.current_frame = 0;
                        a.frame_elapsed = 0;
                        a.state_elapsed = 0;
                }

                anim_params_t p;
                p.speed = (!dead && ac.state == ActionStateComponent::WALK) ? 1.0f : 0.0f;
                p.dir = ac.dir;
                // 攻击 trigger：attack_timer 刚被置满的那一帧才触发一次
                p.attack = (!dead && ac.state == ActionStateComponent::ATTACK &&
                        ac.attack_timer == ActionStateComponent::ATTACK_DURATION);
                // 击退 trigger：knockback_timer 刚被置满的那一帧才触发一次
                p.knocked = (!dead && ac.knockback_timer == ActionStateComponent::KNOCKED_DURATION);

                anim_update(&a, &def, &p, dt);

                // 朝向：bomb_buy 图集默认朝右（survivor_a 朝左，已弃用），故向左走才需要水平翻转。
                // 上/下沿用上一次的朝向，避免上下走时左右抖动。
                // dir: 0=下 1=左 2=右 3=上（见 input_to_dir）
                if (ac.dir == 1)      a.flip_h = true;    // 左：镜像朝左
                else if (ac.dir == 2) a.flip_h = false;   // 右：图集默认朝右，无需翻转
                // 上/下：保持 a.flip_h 不变

                // ---- 脚部粒子（跑/跳/落尘土，纯渲染）----
                // 垂直速度来自 LogicVelocityComponent.y：负=起跳、0=地面、正=下落。
                auto& ppc = registry.get_or_emplace<PlayerParticleComponent>(e);
                float back = (ac.dir == 1) ? 1.0f : (ac.dir == 2) ? -1.0f : 0.0f;
                float feet_x = t.position.x;
                // 盒底：盒高随渲染放大（1.0 → kEntityRenderScale 倍），半高即盒底偏移。
                float feet_y = t.position.y + 0.5f * kEntityRenderScale;
                if (!dead) {
                    if (grounded && ac.state == ActionStateComponent::WALK) {
                        // 按时间累积发射（每秒 kRunEmitRate 个），与渲染帧率无关：
                        // 原先按帧 50% 概率，60fps≈30/s、144fps≈72/s，叠加后尘量过大。
                        const float kRunEmitRate = 2.5f;
                        ppc.run_emit_acc += dt * kRunEmitRate;
                        while (ppc.run_emit_acc >= 1.0f) {
                            ppc.run_emit_acc -= 1.0f;
                            player_emit_particle(ppc, feet_x, feet_y, 0, back);   // 跑
                        }
                    } else {
                        ppc.run_emit_acc = 0.0f;   // 停止移动时不累积，避免起步瞬间喷一大团
                    }
                    if (ppc.prev_vy > -0.5f && pv_y < -1.0f) {                    // 起跳瞬间
                        for (int k = 0; k < 6; k++)
                            player_emit_particle(ppc, feet_x, feet_y, 1, back);
                    }
                    // 落地瞬间才冒尘（上一帧还在下落、本帧着地）。
                    // 不做"持续下落尾迹"：下落快时粒子会被拉成一条白气尾巴。
                    if (grounded && ppc.prev_vy > 2.0f) {
                        for (int k = 0; k < 5; k++)
                            player_emit_particle(ppc, feet_x, feet_y, 2, back);
                    }
                }
                ppc.prev_vy = pv_y;
                player_update_draw_particles(ctx, gs_p, ppc, z, dt);   // 画在角色之下

                // ---- 绘制 ----
                anim_clip_t clip;
                if (!anim_get_current_clip(&a, &def, &clip)) continue;

                image_p img = sprite_get_image(ctx, clip.path);
                if (!img) continue;

                // 逻辑坐标(米) → 屏幕像素（含世界缩放）
                float sx = meters_to_pixels(t.position.x - cam.x) * z;
                float sy = meters_to_pixels(t.position.y - cam.y) * z;

                // 素材帧高 32px；按 50 PPM（含缩放）缩放到目标渲染高度。
                // 乘 kEntityRenderScale：64×64 图集下让角色约占 1.5 格，与地图比例协调。
                const float scale = (clip.src_rect.h > 0.0f)
                        ? (kEntityRenderScale * 50.0f * z / clip.src_rect.h) : 1.0f;
                vec2_t pos = { sx, sy };
                // image_draw_ex 会把负 scale 转为 SDL_FLIP_HORIZONTAL（见 jr2d.c），
                // 因此朝右时传负的 x 缩放即可实现镜像，无需引擎新增 flip 参数。
                vec2_t scl = { a.flip_h ? -scale : scale, scale };
                vec2_t origin = { clip.src_rect.w * 0.5f, clip.src_rect.h * 0.5f };

                image_draw_ex(img, &clip.src_rect, pos, 0.0f, scl, origin);
        }
}

#endif