#include "editorGizmoOverlay.hpp"

#include <cmath>
#include <limits>

#include <glm.hpp>
#include <imgui.h>

#include "camera.hpp"
#include "component.hpp"
#include "editorSelection.hpp"
#include "object.hpp"
#include "scene.hpp"

namespace
{
struct ProjectedPoint
{
    ImVec2 position;
    float depth = 0.0f;
};

bool ProjectWorldPoint(const glm::vec3 &worldPosition, const glm::mat4 &viewProjection, const ImVec2 &imageMinimum, const ImVec2 &imageMaximum, ProjectedPoint &projected)
{
    const glm::vec4 clip = viewProjection * glm::vec4(worldPosition, 1.0f);

    if (clip.w <= 0.0001f) {
        return false;
    }

    const glm::vec3 normalised = glm::vec3(clip) / clip.w;

    if (normalised.x < -1.0f || normalised.x > 1.0f || normalised.y < -1.0f || normalised.y > 1.0f || normalised.z < -1.0f || normalised.z > 1.0f) {
        return false;
    }

    const float width = imageMaximum.x - imageMinimum.x;

    const float height = imageMaximum.y - imageMinimum.y;

    projected.position.x = imageMinimum.x + (normalised.x * 0.5f + 0.5f) * width;

    projected.position.y = imageMinimum.y + (1.0f - (normalised.y * 0.5f + 0.5f)) * height;

    projected.depth = normalised.z;
    return true;
}

void DrawCameraIcon(ImDrawList &drawList, const ImVec2 &centre, ImU32 colour)
{
    // Draw a larger body so the icon remains legible at typical zoom levels.
    drawList.AddRectFilled(ImVec2(centre.x - 10.0f, centre.y - 6.5f), ImVec2(centre.x + 6.0f, centre.y + 6.5f), colour, 2.5f);

    // Lens is anchored directly to the body edge so the two shapes read as one icon.
    const ImVec2 lensPoints[4] = {ImVec2(centre.x + 6.0f, centre.y - 4.5f), ImVec2(centre.x + 6.0f, centre.y + 4.5f), ImVec2(centre.x + 13.0f, centre.y + 8.0f), ImVec2(centre.x + 13.0f, centre.y - 8.0f)};

    drawList.AddConvexPolyFilled(lensPoints, 4, colour);
}

void DrawLightIcon(ImDrawList &drawList, const ImVec2 &centre, ImU32 colour)
{
    drawList.AddCircleFilled(centre, 5.0f, colour, 16);

    constexpr float Pi = 3.14159265358979323846f;

    for (int ray = 0; ray < 8; ++ray) {
        const float angle = static_cast<float>(ray) * Pi * 0.25f;

        const ImVec2 inner(centre.x + std::cos(angle) * 8.0f, centre.y + std::sin(angle) * 8.0f);

        const ImVec2 outer(centre.x + std::cos(angle) * 12.0f, centre.y + std::sin(angle) * 12.0f);

        drawList.AddLine(inner, outer, colour, 1.5f);
    }
}

void DrawObjectGizmos(Object &object, const glm::mat4 &viewProjection, const ImVec2 &imageMinimum, const ImVec2 &imageMaximum, EditorSelection &selection, ImDrawList &drawList, const ImVec2 &mousePosition, Object *&hoveredObject, ImVec2 &hoveredPosition, float &closestDistanceSquared)
{
    if (!object.isActive()) {
        return;
    }

    Component *cameraComponent = object.getComponentByTypeName("Camera");

    Component *lightComponent = object.getComponentByTypeName("PointLight2D");

    const bool showCamera = cameraComponent != nullptr && cameraComponent->isEnabled();

    const bool showLight = lightComponent != nullptr && lightComponent->isEnabled();

    if (showCamera || showLight) {
        ProjectedPoint projected;

        if (ProjectWorldPoint(object.getWorldPosition(), viewProjection, imageMinimum, imageMaximum, projected)) {
            if (showCamera) {
                DrawCameraIcon(drawList, projected.position, IM_COL32(75, 170, 255, 255));
            }
            else {
                DrawLightIcon(drawList, projected.position, IM_COL32(255, 210, 65, 255));
            }

            if (selection.isSelected(object.getId())) {
                drawList.AddCircle(projected.position, 20.0f, IM_COL32(255, 255, 255, 255), 24, 2.0f);
            }

            const float differenceX = mousePosition.x - projected.position.x;

            const float differenceY = mousePosition.y - projected.position.y;

            const float distanceSquared = differenceX * differenceX + differenceY * differenceY;

            if (distanceSquared <= 20.0f * 20.0f && distanceSquared < closestDistanceSquared) {
                closestDistanceSquared = distanceSquared;

                hoveredObject = &object;
                hoveredPosition = projected.position;
            }
        }
    }

    for (std::size_t childIndex = 0; childIndex < object.getChildCount(); ++childIndex) {
        Object *child = object.getChild(childIndex);

        if (child != nullptr) {
            DrawObjectGizmos(*child, viewProjection, imageMinimum, imageMaximum, selection, drawList, mousePosition, hoveredObject, hoveredPosition, closestDistanceSquared);
        }
    }
}
} // namespace

bool DrawEditorGizmoOverlay(Scene &scene, EditorSelection &selection, const Camera &camera, float imageMinimumX, float imageMinimumY, float imageMaximumX, float imageMaximumY, bool viewportHovered)
{
    if (imageMaximumX <= imageMinimumX || imageMaximumY <= imageMinimumY) {
        return false;
    }

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    if (drawList == nullptr) {
        return false;
    }

    const ImVec2 imageMinimum(imageMinimumX, imageMinimumY);

    const ImVec2 imageMaximum(imageMaximumX, imageMaximumY);

    const ImVec2 mousePosition = ImGui::GetIO().MousePos;

    Object *hoveredObject = nullptr;
    ImVec2 hoveredPosition;
    bool clickConsumed = false;

    float closestDistanceSquared = std::numeric_limits<float>::max();

    drawList->PushClipRect(imageMinimum, imageMaximum, true);

    const glm::mat4 viewProjection = camera.getViewProjectionMatrix();

    for (std::size_t rootIndex = 0; rootIndex < scene.getRootObjectCount(); ++rootIndex) {
        Object *root = scene.getRootObject(rootIndex);

        if (root != nullptr) {
            DrawObjectGizmos(*root, viewProjection, imageMinimum, imageMaximum, selection, *drawList, mousePosition, hoveredObject, hoveredPosition, closestDistanceSquared);
        }
    }

    if (viewportHovered && hoveredObject != nullptr) {
        drawList->AddCircle(hoveredPosition, 22.0f, IM_COL32(125, 205, 255, 255), 24, 2.0f);

        ImGui::SetTooltip("%s", hoveredObject->getName().c_str());

        if (ImGui::IsMouseClicked(0)) {
            selection.selectObject(hoveredObject);

            clickConsumed = true;
        }
    }

    drawList->PopClipRect();
    return clickConsumed;
}