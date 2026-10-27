#pragma once

struct SpriteSheetComponent {
    char path[256];
    int frame_w;
    int frame_h;
    int idle_row;       // idle 动画行
    int walk_row;       // walk 动画行
    int atk_rows[4];    // attack 动画行 [下,左,右,上]
    int frame_count;    // 每行动画帧数
    float frame_duration;
};

struct AnimationFrameComponent {
    int cur_row;
    int cur_frame;
    float timer;
};
