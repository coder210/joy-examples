#include "debug_layer.h"
#include <joy/joy.h>

DebugLayer::DebugLayer(Context* ctx) : UILayer(ctx) {}

DebugLayer::~DebugLayer() = default;

void DebugLayer::Render()
{
        drawDebug();
}

void DebugLayer::Destroy() {}

// 原 on_render：在节点 world position（0,0）画一个 60x60 方块，颜色由 zorder 决定；
// debug_layer 节点 zorder 默认 0 → 红色。现已按需求去掉该红色方块可视化。
void DebugLayer::drawDebug()
{
        // 不再绘制左上角红色调试方块。
}
