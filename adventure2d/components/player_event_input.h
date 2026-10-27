#ifndef PLAYER_EVENT_INPUT_H
#define PLAYER_EVENT_INPUT_H
#include <vector>
#include <joy/jmath.h>

struct PlayerEventInput {
    // 单个动作定义
    struct Action {
        uint32_t client_seq;   // 客户端序列号（用于调试/防重放）
        uint8_t type;          // 0=普通攻击, 1=技能1, 2=技能2, 3=终极技...
        // 可扩展：如目标位置、角度等
    };

    // 待处理的事件队列（FIFO）
    std::vector<Action> pending_actions;

    // 方便清空
    void clear() { pending_actions.clear(); }

};

#endif
