#include <renderer/common/RenderBatchBuilder.h>

namespace xpf {

void RenderBatchBuilder::DrawCircle(float x, float y, float radius, xpf::Color color)
{
    RectangleDescription description;
    description.borderColor = color;
    description.cornerRadius = CornerRadius(radius);
    description.fillColor = color;
    description.height = radius + radius;
    description.width = radius + radius;
    DrawRectangle(x - radius, y - radius, description);
}

void RenderBatchBuilder::DrawCircle(float x, float y, const CircleDescription& cdescription)
{
    RectangleDescription description;
    description.borderColor = cdescription.borderColor;
    description.borderThickness = cdescription.borderThickness;
    description.cornerRadius = CornerRadius(cdescription.radius);
    description.fillColor = cdescription.fillColor;
    description.height = cdescription.radius + cdescription.radius;
    description.width = cdescription.radius + cdescription.radius;
    DrawRectangle(x - cdescription.radius, y - cdescription.radius, description);
}

} // xpf