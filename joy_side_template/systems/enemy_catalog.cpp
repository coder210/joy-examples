#include "enemy_catalog.h"
#include "../asset_manager.h"
#include <joy/external/cJSON.h>
#include <cstdio>
#include <cstring>

// 移速安全区间（米/秒）。上限 15 保证单 tick 位移 < 1 格（20Hz 下 step<0.75m），
// 否则离散碰撞检测可能漏检穿墙。两端施加同一钳制，帧同步仍然一致。
static const float kEnemySpeedMin = 0.5f;
static const float kEnemySpeedMax = 15.0f;

// anims 键（顺序对应 EnemyAnimId）
static const char* kEnemyAnimKeys[ENEMY_ANIM_COUNT] = {
    "idle", "run", "attack", "hit", "dead", "dead_ground"
};

// 内置默认表：enemies.json 缺失/解析失败时的兜底。
// 必须与 enemies.json 中 bald_pirate 的语义保持一致，且两端相同。
void EnemyCatalog::setBuiltinDefault()
{
    defs_.clear();
    byKey_.clear();
    EnemyDef d;
    d.id   = 0;
    d.key  = "bald_pirate";
    d.name = "光头海盗";
    d.max_hp = 50;
    d.move_speed = fp_from_float(3.0f);
    d.melee_damage = 20;
    d.ranged = false;
    d.patrol_radius = 3.0f;
    d.detect_radius = 6.0f;
    d.attack_radius = 1.5f;
    d.sprite_dir = "joy_side_template/sprites/enemies/bald_pirate/";
    d.anims[ENEMY_ANIM_IDLE].dir        = "1-Idle";         d.anims[ENEMY_ANIM_IDLE].frames = 34;
    d.anims[ENEMY_ANIM_RUN].dir         = "2-Run";          d.anims[ENEMY_ANIM_RUN].frames = 14;
    d.anims[ENEMY_ANIM_ATTACK].dir      = "7-Attack";       d.anims[ENEMY_ANIM_ATTACK].frames = 12;
    d.anims[ENEMY_ANIM_HIT].dir         = "8-Hit";          d.anims[ENEMY_ANIM_HIT].frames = 8;
    d.anims[ENEMY_ANIM_DEAD].dir        = "9-Dead Hit";     d.anims[ENEMY_ANIM_DEAD].frames = 6;
    d.anims[ENEMY_ANIM_DEAD_GROUND].dir = "10-Dead Ground"; d.anims[ENEMY_ANIM_DEAD_GROUND].frames = 4;
    defs_.push_back(d);
    byKey_[d.key] = d.id;
}

static bool enemy_read_file_text(const char* path, std::string& out)
{
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return false;
    std::fseek(f, 0, SEEK_END);
    long len = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (len <= 0) { std::fclose(f); return false; }
    out.assign((size_t)len, '\0');
    size_t got = std::fread(&out[0], 1, (size_t)len, f);
    std::fclose(f);
    return got == (size_t)len;
}

void EnemyCatalog::load()
{
    setBuiltinDefault();   // 先装兜底，解析成功则整体替换

    const char* path = AssetManager::GetInstance()
        ? AssetManager::GetInstance()->GetResourcePath(AssetManager::kCatConfig, "enemies")
        : nullptr;
    if (!path) {
        log_info("[enemy] no config.enemies entry, using built-in defaults");
        return;
    }

    std::string text;
    if (!enemy_read_file_text(path, text)) {
        log_info("[enemy] %s not found, using built-in defaults", path);
        return;
    }

    cJSON* root = cJSON_Parse(text.c_str());
    if (!root) {
        log_error("[enemy] parse %s failed, using built-in defaults", path);
        return;
    }

    std::vector<EnemyDef> parsed;
    int next_id = 0;

    // cJSON v1.x 无 cJSON_ArrayForEach：按 child 链表遍历，顺序即文件中的出现顺序。
    for (cJSON* ch = root->child; ch; ch = ch->next) {
        if (ch->type != cJSON_Object || !ch->string) continue;

        EnemyDef d;
        d.id  = next_id++;
        d.key = ch->string;

        cJSON* v = cJSON_GetObjectItem(ch, "name");
        d.name = (v && v->type == cJSON_String && v->valuestring) ? v->valuestring : d.key;

        v = cJSON_GetObjectItem(ch, "sprite_dir");
        if (v && v->type == cJSON_String && v->valuestring) d.sprite_dir = v->valuestring;

        // 素材默认朝向：faces_left=true 表示精灵本身朝左（镜像判定反转）。
        v = cJSON_GetObjectItem(ch, "faces_left");
        if (v) {
            if (v->type == cJSON_True)       d.src_faces_left = true;
            else if (v->type == cJSON_False) d.src_faces_left = false;
        }

        cJSON* stats = cJSON_GetObjectItem(ch, "stats");
        if (stats) {
            cJSON* hp = cJSON_GetObjectItem(stats, "hp");
            if (hp && hp->type == cJSON_Number) d.max_hp = (int)hp->valuedouble;
            cJSON* sp = cJSON_GetObjectItem(stats, "speed");
            if (sp && sp->type == cJSON_Number) d.move_speed = fp_from_float((float)sp->valuedouble);
            cJSON* md = cJSON_GetObjectItem(stats, "melee_damage");
            if (md && md->type == cJSON_Number) d.melee_damage = (int)md->valuedouble;
            cJSON* rh = cJSON_GetObjectItem(stats, "render_height");
            if (rh && rh->type == cJSON_Number) d.render_height = (float)rh->valuedouble;
        }

        cJSON* ai = cJSON_GetObjectItem(ch, "ai");
        if (ai) {
            // 注意：本工程内嵌的 cJSON 是 v1.x，只有 cJSON_True / cJSON_False，
            // 没有 cJSON_Bool，不能用 cJSON_Bool 判定布尔。
            cJSON* rg = cJSON_GetObjectItem(ai, "ranged");
            if (rg) {
                if (rg->type == cJSON_True)       d.ranged = true;
                else if (rg->type == cJSON_False) d.ranged = false;
            }

            cJSON* pr = cJSON_GetObjectItem(ai, "patrol_radius");
            if (pr && pr->type == cJSON_Number) d.patrol_radius = (float)pr->valuedouble;
            cJSON* dr = cJSON_GetObjectItem(ai, "detect_radius");
            if (dr && dr->type == cJSON_Number) d.detect_radius = (float)dr->valuedouble;
            cJSON* ar = cJSON_GetObjectItem(ai, "attack_radius");
            if (ar && ar->type == cJSON_Number) d.attack_radius = (float)ar->valuedouble;
            cJSON* bs = cJSON_GetObjectItem(ai, "bullet_speed");
            if (bs && bs->type == cJSON_Number) d.bullet_speed = (float)bs->valuedouble;
            cJSON* bi = cJSON_GetObjectItem(ai, "bullet_interval");
            if (bi && bi->type == cJSON_Number) d.bullet_interval = (float)bi->valuedouble;
            cJSON* bd = cJSON_GetObjectItem(ai, "bullet_damage");
            if (bd && bd->type == cJSON_Number) d.bullet_damage = (float)bd->valuedouble;
        }

        cJSON* anims = cJSON_GetObjectItem(ch, "anims");
        if (anims) {
            for (int a = 0; a < ENEMY_ANIM_COUNT; a++) {
                cJSON* av = cJSON_GetObjectItem(anims, kEnemyAnimKeys[a]);
                if (!av) continue;
                cJSON* dv = cJSON_GetObjectItem(av, "dir");
                if (dv && dv->type == cJSON_String && dv->valuestring)
                    d.anims[a].dir = dv->valuestring;
                cJSON* fv = cJSON_GetObjectItem(av, "frames");
                if (fv && fv->type == cJSON_Number) d.anims[a].frames = (int)fv->valuedouble;
            }
        }

        parsed.push_back(d);
        if ((int)parsed.size() >= MAX_ENEMIES) break;
    }
    cJSON_Delete(root);

    if (parsed.empty()) {
        log_error("[enemy] %s has no enemy entry, using built-in defaults", path);
        return;
    }

    // 移速钳制：异常值会导致瞬移/穿墙。两端同样钳制，结果仍一致。
    for (size_t i = 0; i < parsed.size(); i++) {
        float sp = fp_to_float(parsed[i].move_speed);
        if (sp < kEnemySpeedMin || sp > kEnemySpeedMax) {
            log_error("[enemy] '%s' speed=%.2f m/s out of range [%.1f,%.1f], clamped",
                parsed[i].key.c_str(), sp, kEnemySpeedMin, kEnemySpeedMax);
            parsed[i].move_speed = fp_from_float(sp < kEnemySpeedMin ? kEnemySpeedMin : kEnemySpeedMax);
        }
    }

    defs_.swap(parsed);
    byKey_.clear();
    for (const auto& d : defs_) byKey_[d.key] = d.id;
    log_info("[enemy] loaded %s: %d enemy type(s)", path, (int)defs_.size());
}

const EnemyDef& EnemyCatalog::get(int id)
{
    if (defs_.empty()) setBuiltinDefault();
    if (id < 0 || id >= (int)defs_.size()) id = 0;
    return defs_[id];
}

const EnemyDef* EnemyCatalog::findByName(const std::string& key)
{
    if (defs_.empty()) setBuiltinDefault();
    auto it = byKey_.find(key);
    if (it == byKey_.end()) return nullptr;
    return &defs_[it->second];
}

int EnemyCatalog::frameCount(const EnemyDef& def, EnemyAnimId anim) const
{
    if (anim < 0 || anim >= ENEMY_ANIM_COUNT) anim = ENEMY_ANIM_IDLE;
    const EnemyAnim& a = def.anims[anim];
    if (a.frames > 0 && !a.dir.empty()) return a.frames;
    const EnemyAnim& idle = def.anims[ENEMY_ANIM_IDLE];   // 素材缺失 → 降级
    return (idle.frames > 0 && !idle.dir.empty()) ? idle.frames : 0;
}

std::string EnemyCatalog::framePath(const EnemyDef& def, EnemyAnimId anim, int frame) const
{
    if (anim < 0 || anim >= ENEMY_ANIM_COUNT) anim = ENEMY_ANIM_IDLE;
    const EnemyAnim* a = &def.anims[anim];
    if (a->frames <= 0 || a->dir.empty()) a = &def.anims[ENEMY_ANIM_IDLE];  // 降级 idle
    if (a->frames <= 0 || a->dir.empty()) return std::string();

    int f = frame % a->frames;
    if (f < 0) f += a->frames;

    std::string p = def.sprite_dir;
    if (!p.empty() && p[p.size() - 1] != '/') p += "/";
    p += a->dir;
    p += "/";
    p += std::to_string(f + 1);   // 帧文件从 1.png 起
    p += ".png";
    return p;
}
