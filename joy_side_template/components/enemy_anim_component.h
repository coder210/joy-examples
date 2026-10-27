#pragma once
// 敌人序列帧动画的运行时状态（每实体一份）。
// 与玩家的 AnimStateComponent（janim 状态机）不同：敌人素材是"每个动画一个文件夹、
// 内含 1.png..N.png 逐帧"，而非横向精灵表，故这里用更轻量的「动画槽 + 帧号 + 计时」自管推进。
// 纯渲染表现：不参与帧同步、不进快照、绝不回写逻辑组件。
struct EnemyAnimComponent {
    int   anim    = 0;      // 当前播放的动画槽（EnemyAnimId）
    int   frame   = 0;      // 当前帧序号（0 基）
    float elapsed = 0.0f;   // 当前帧已播放时长（秒）
    bool  flip_h  = false;  // 水平镜像（朝左时置 true）
    float total   = 0.0f;   // 当前动画已播放总时长（秒），用于死亡动画播完判定
    // 一次性动画（attack/hit/dead/dead_ground）是否已播到末帧。
    // 死亡表现用：Dead Hit → Dead Ground 整段播完后渲染层不再绘制该实体（视觉"消失"），
    // 但实体仍以 dead 保留在逻辑世界（胜利判定 / checksum 不受影响，见 draw_enemy_pass）。
    bool  finished = false;

    // ---- 受击表现（渲染侧自检测，不改逻辑组件）----
    // 敌人被玩家打时，逻辑侧并没有给它置"硬直"标记（那是玩家专属的 knockback_timer），
    // 且 ActionStateComponent.knockback_timer 的递减只写在玩家分支里（input_consume_input_system），
    // 给敌人置位会导致它永不递减、卡在受击态。
    // 故改为渲染侧比对血量：hp 下降即触发一次受击动画，纯表现、零帧同步风险。
    int   prev_hp  = -1;    // 上一渲染帧的 hp（-1 = 尚未采样）
    float hit_time = 0.0f;  // 剩余受击表现时长（秒），>0 时播 Hit 动画
};
