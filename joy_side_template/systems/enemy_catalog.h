#ifndef ENEMY_CATALOG_H
#define ENEMY_CATALOG_H
#include <string>
#include <vector>
#include <unordered_map>
#include <joy/jmath.h>

// 敌人表：唯一真源是 data/enemies.json（经 resources.json 的 config.enemies 索引）。
// 本文件只做"运行时 catalog"——解析、缓存、按整数 id / key 查询，不再内嵌任何敌人数值。
//
// 敌人上限：防止 JSON 异常时敌人表无限增长。
static const int MAX_ENEMIES = 16;

// 敌人动画槽（对应 enemies.json 的 anims 键）。
// 素材格式与玩家不同：玩家是横向精灵表（单张 png，帧数=宽/高），
// 敌人是"每个动画一个文件夹，内含 1.png .. N.png 逐帧"。
// 且各敌人的文件夹编号不一致（如 Hit 在 bald_pirate 是 8-Hit、big_guy 是 12-Hit），
// 故必须由 JSON 显式给出每个动画的文件夹名与帧数，不能按顺序下标推导。
enum EnemyAnimId {
    ENEMY_ANIM_IDLE        = 0,
    ENEMY_ANIM_RUN         = 1,
    ENEMY_ANIM_ATTACK      = 2,
    ENEMY_ANIM_HIT         = 3,
    ENEMY_ANIM_DEAD        = 4,
    ENEMY_ANIM_DEAD_GROUND = 5,
    ENEMY_ANIM_COUNT       = 6,
};

// 单个动画：文件夹名 + 帧数（帧文件为 1.png .. frames.png）
struct EnemyAnim {
    std::string dir;
    int         frames = 0;
};

struct EnemyDef {
    int         id = 0;              // 整数 id，按 JSON 中出现顺序分配（协议/存档用）
    std::string key;                 // "bald_pirate"
    std::string name;                // 展示名

    // ---- 数值 ----
    int         max_hp = 50;         // stats.hp
    fp_t        move_speed = 0;      // stats.speed（米/秒，已定点化）
    int         melee_damage = 0;    // stats.melee_damage（近战伤害）
    float       render_height = 1.0f;// stats.render_height（渲染高度，米；体型区分，纯表现）
    // 素材默认朝向：false=默认朝右(原行为，dir==1 左才镜像)；true=素材本身朝左，
    // 镜像条件反转（dir==2 右才镜像）。whale 等素材朝左需在 enemies.json 标 faces_left:true。
    bool        src_faces_left = false;

    // ---- AI ----
    bool        ranged = false;      // 是否远程敌人
    float       patrol_radius = 3.0f;
    float       detect_radius = 6.0f;
    float       attack_radius = 1.5f;
    float       bullet_speed = 0.0f;
    float       bullet_interval = 0.0f;   // 秒
    float       bullet_damage = 0.0f;

    std::string sprite_dir;          // 精灵根目录（运行时相对路径）
    EnemyAnim   anims[ENEMY_ANIM_COUNT];
};

// 运行时敌人目录：解析、缓存、按 id / key 查询。唯一真源是 data/enemies.json。
class EnemyCatalog {
public:
    // 解析 enemies.json 填充敌人表。必须在 AssetManager::Init() 之后调用。
    // 文件缺失/解析失败时回退内置默认表——两端回退到同一份默认值，帧同步仍然一致。
    void load();

    // 按整数 id 取敌人定义；越界回退 id=0。至少含 1 项。
    const EnemyDef& get(int id);

    // 按 key 字符串（如 "bald_pirate"）反查；找不到返回 nullptr。
    const EnemyDef* findByName(const std::string& key);

    size_t size() const { return defs_.size(); }

    // 取该动画的第 frame 帧（0 基）的完整运行时路径：
    // sprite_dir + anim.dir + "/" + (frame+1) + ".png"
    // 该动画缺失（dir 为空或 frames<=0）时自动降级到 idle；连 idle 都不可用则返回空串。
    std::string framePath(const EnemyDef& def, EnemyAnimId anim, int frame) const;

    // 该动画的帧数（降级后为 idle 的帧数，可能为 0）
    int frameCount(const EnemyDef& def, EnemyAnimId anim) const;

private:
    void setBuiltinDefault();                        // 兜底内置表（bald_pirate）

    std::vector<EnemyDef> defs_;                     // 稠密，按 id 索引
    std::unordered_map<std::string, int> byKey_;     // key → id 反查
};

// 全局唯一敌人目录实例
inline EnemyCatalog g_enemyCatalog;

#endif
