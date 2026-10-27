#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H
#include <joy/joy.h>

typedef struct debug_layer debug_layer_t, *debug_layer_p;

debug_layer_p create_debug_layer();
scene_node_p debug_layer_get_node(debug_layer_p debug_layer);

#endif