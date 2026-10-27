#ifndef GAME_MODE_H
#define GAME_MODE_H

struct game_scene;
typedef struct game_scene* game_scene_p;

// 模式特有接口：将客户端/服务端/单机/Host 的特有逻辑抽离到 client.cpp / server.cpp 中
typedef struct {
    // 模式初始化：网络创建、音频、玩家注入、系统注册
    void (*init)(game_scene_p);
    // 事件处理
    void (*handle_event)(game_scene_p, const void*);
    // 网络接收（tick 前）
    void (*network_update)(game_scene_p);
    // 单 tick 的输入侧逻辑：帧号分配、输入收集、命令打包/发送
    void (*tick)(game_scene_p);
    // 模式特有渲染
    void (*render)(game_scene_p);
    // 清理
    void (*destroy)(game_scene_p);
} GameModeInterface;

extern GameModeInterface g_single_mode;
extern GameModeInterface g_client_mode;
extern GameModeInterface g_host_mode;
extern GameModeInterface g_server_mode;

#endif