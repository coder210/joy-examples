#ifndef GAMEPLAYER_CONTROLS_LAYER_H
#define GAMEPLAYER_CONTROLS_LAYER_H
#include <joy/joy.h>
#include "../context.h"

#define GAMEPLAY_CONTROL_EVENT (0x1)

enum gameplay_control_event_k {
        GAMEPLAY_CONTROL_NONE = 1 << 0,
        GAMEPLAY_CONTROL_UP = 1 << 1,
        GAMEPLAY_CONTROL_DOWN = 1 << 2,
        GAMEPLAY_CONTROL_LEFT = 1 << 3,
        GAMEPLAY_CONTROL_RIGHT = 1 << 4,
        GAMEPLAY_CONTROL_ATTACK = 1 << 5,
        GAMEPLAY_CONTROL_RELEASE_UP = 1 << 6,
        GAMEPLAY_CONTROL_RELEASE_DOWN = 1 << 7,
        GAMEPLAY_CONTROL_RELEASE_LEFT = 1 << 8,
        GAMEPLAY_CONTROL_RELEASE_RIGHT = 1 << 9,
};

typedef struct gameplay_controls_layer gameplay_controls_layer_t, * gameplay_controls_layer_p;

gameplay_controls_layer_p create_gameplay_controls_layer(Context* ctx);
scene_node_p gameplay_controls_layer_get_node(gameplay_controls_layer_p gameplay_controls_layer);

#endif