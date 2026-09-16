#include "sceneViewPanel.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>

#include <imgui.h>
#include <glm/gtc/matrix_inverse.hpp>

#include "editorCamera.hpp"
#include "editorSelection.hpp"
#include "meshFilterComponent.hpp"
#include "object.hpp"
#include "renderTarget.hpp"
#include "scene.hpp"

#include "editorGizmoOverlay.hpp"

namespace
{
    struct Ray3D
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    bool BuildPickRay(
        const ImVec2& mousePosition,
        const ImVec2& imageMinimum,
        const ImVec2& imageMaximum,
        const Camera3D& camera,
        Ray3D& ray
    )
    {
        const float width = imageMaximum.x - imageMinimum.x;
        const float height = imageMaximum.y - imageMinimum.y;

        if (width <= 0.0f || height <= 0.0f)
        {
            return false;
        }

        const float viewportX =
            (mousePosition.x - imageMinimum.x) / width;

        const float viewportY =
            (mousePosition.y - imageMinimum.y) / height;

        const float ndcX = viewportX * 2.0f - 1.0f;
        const float ndcY = 1.0f - viewportY * 2.0f;

        const glm::mat4 inverseViewProjection =
            glm::inverse(
                camera.getProjectionMatrix() *
                camera.getViewMatrix()
            );

        const glm::vec4 nearPoint =
            inverseViewProjection *
            glm::vec4(ndcX, ndcY, -1.0f, 1.0f);

        const glm::vec4 farPoint =
            inverseViewProjection *
            glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

        if (
            std::abs(nearPoint.w) <= 0.000001f ||
            std::abs(farPoint.w) <= 0.000001f
        )
        {
            return false;
        }

        const glm::vec3 nearWorld =
            glm::vec3(nearPoint) / nearPoint.w;

        const glm::vec3 farWorld =
            glm::vec3(farPoint) / farPoint.w;

        const glm::vec3 direction = farWorld - nearWorld;

        if (glm::dot(direction, direction) <= 0.000001f)
        {
            return false;
        }

        ray.origin = nearWorld;
        ray.direction = glm::normalize(direction);

        return true;
    }

    bool IntersectSphere(
        const Ray3D& ray,
        const glm::vec3& centre,
        float radius,
        float& hitDistance
    )
    {
        const glm::vec3 offset = ray.origin - centre;

        const float a = glm::dot(ray.direction, ray.direction);
        const float b = 2.0f * glm::dot(offset, ray.direction);
        const float c = glm::dot(offset, offset) - radius * radius;

        const float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f)
        {
            return false;
        }

        const float squareRoot = std::sqrt(discriminant);
        const float inverseDenominator = 1.0f / (2.0f * a);

        const float nearHit =
            (-b - squareRoot) * inverseDenominator;

        const float farHit =
            (-b + squareRoot) * inverseDenominator;

        if (farHit < 0.0f)
        {
            return false;
        }

        hitDistance = nearHit >= 0.0f ? nearHit : farHit;
        return true;
    }

    bool IntersectUnitCube(
        const Ray3D& ray,
        const glm::mat4& worldMatrix,
        float& hitDistance
    )
    {
        if (std::abs(glm::determinant(worldMatrix)) <= 0.000001f)
        {
            return false;
        }

        const glm::mat4 inverseWorld = glm::inverse(worldMatrix);

        const glm::vec3 localOrigin =
            glm::vec3(inverseWorld * glm::vec4(ray.origin, 1.0f));

        const glm::vec3 localDirection =
            glm::normalize(
                glm::vec3(inverseWorld * glm::vec4(ray.direction, 0.0f))
            );

        float entryDistance = -std::numeric_limits<float>::infinity();
        float exitDistance = std::numeric_limits<float>::infinity();

        for (int axis = 0; axis < 3; ++axis)
        {
            const float origin = localOrigin[axis];
            const float direction = localDirection[axis];

            if (std::abs(direction) <= 0.000001f)
            {
                if (origin < -0.5f || origin > 0.5f)
                {
                    return false;
                }

                continue;
            }

            const float inverseDirection = 1.0f / direction;

            float axisEntry = (-0.5f - origin) * inverseDirection;
            float axisExit = (0.5f - origin) * inverseDirection;

            if (axisEntry > axisExit)
            {
                std::swap(axisEntry, axisExit);
            }

            entryDistance = std::max(entryDistance, axisEntry);
            exitDistance = std::min(exitDistance, axisExit);

            if (entryDistance > exitDistance)
            {
                return false;
            }
        }

        if (exitDistance < 0.0f)
        {
            return false;
        }

        hitDistance =
            entryDistance >= 0.0f
                ? entryDistance
                : exitDistance;

        return true;
    }

    void FindPickedObject(
        Object& object,
        const Ray3D& ray,
        Object*& bestObject,
        float& bestHitDistance
    )
    {
        if (object.isActive())
        {
            float hitDistance = 0.0f;

            const bool hasMesh =
                object.getComponent<
                    MeshFilterComponent
                >() != nullptr;

            const bool hit = hasMesh
                ? IntersectUnitCube(
                    ray,
                    object.getWorldMatrix(),
                    hitDistance
                )
                : IntersectSphere(
                    ray,
                    object.getWorldPosition(),
                    0.55f,
                    hitDistance
                );

            if (hit && hitDistance < bestHitDistance)
            {
                bestHitDistance = hitDistance;
                bestObject = &object;
            }
        }

        for (
            std::size_t childIndex = 0;
            childIndex < object.getChildCount();
            ++childIndex
        )
        {
            Object* child = object.getChild(childIndex);

            if (child != nullptr)
            {
                FindPickedObject(
                    *child,
                    ray,
                    bestObject,
                    bestHitDistance
                );
            }
        }
    }
}

void DrawSceneViewPanel(const RenderTarget& renderTarget, EditorCamera& editorCamera, Scene& scene,
    EditorSelection& selection,SceneViewPanelState& state)
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (!ImGui::Begin("Scene View", nullptr, flags))
    {
        state.hovered = false;
        state.focused = false;
        ImGui::End();
        return;
    }

    const ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x >= 16.0f && available.y >= 16.0f )
    {
        state.requestedWidth = static_cast<int>(available.x);
        state.requestedHeight = static_cast<int>(available.y);
    }

    state.hovered = false;

    if (renderTarget.isValid() && available.x > 0.0f && available.y > 0.0f)
    {
        const ImVec2 imageMinimum = ImGui::GetCursorScreenPos();

        const ImTextureID texture = static_cast<ImTextureID>(static_cast<intptr_t>(renderTarget.getColourTextureId()));

        ImGui::Image(texture, available, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        const ImVec2 imageMaximum(imageMinimum.x + available.x, imageMinimum.y + available.y);

        state.imageMinimumX = imageMinimum.x;
        state.imageMinimumY = imageMinimum.y;
        state.imageMaximumX = imageMaximum.x;
        state.imageMaximumY = imageMaximum.y;

        state.hovered = ImGui::IsItemHovered();

        state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        const bool gizmoConsumedClick =
            DrawEditorGizmoOverlay(
                scene,
                selection,
                editorCamera.getCamera(),
                imageMinimum.x,
                imageMinimum.y,
                imageMaximum.x,
                imageMaximum.y,
                state.hovered
            );

        if (
            state.hovered &&
            !gizmoConsumedClick &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        )
        {
            Ray3D ray;

            if (
                BuildPickRay(
                    ImGui::GetIO().MousePos,
                    imageMinimum,
                    imageMaximum,
                    editorCamera.getCamera(),
                    ray
                )
            )
            {
                Object* pickedObject = nullptr;

                float bestHitDistance =
                    std::numeric_limits<float>::max();

                for (
                    std::size_t rootIndex = 0;
                    rootIndex < scene.getRootObjectCount();
                    ++rootIndex
                )
                {
                    Object* root = scene.getRootObject(rootIndex);

                    if (root != nullptr)
                    {
                        FindPickedObject(
                            *root,
                            ray,
                            pickedObject,
                            bestHitDistance
                        );
                    }
                }

                if (pickedObject != nullptr)
                {
                    selection.selectObject(pickedObject);
                }
                else
                {
                    selection.clear();
                }
            }
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(imageMinimum, imageMaximum, state.hovered ? IM_COL32(90, 170, 255, 255) : IM_COL32(65, 70, 80, 255));

        const glm::vec3& position = editorCamera.getPosition();

        char positionText[128];

        std::snprintf(
            positionText,
            sizeof(positionText),
            "Scene Camera: %.1f, %.1f, %.1f",
            position.x,
            position.y,
            position.z
        );

        drawList->AddText(ImVec2(imageMinimum.x + 10.0f, imageMinimum.y + 10.0f), IM_COL32(235, 240, 250, 255), positionText);

        drawList->AddText(ImVec2(imageMinimum.x + 10.0f, imageMinimum.y + 30.0f), IM_COL32(190, 195, 205, 255), "RMB + WASD/QE | Shift = Faster | F = Focus");
    }
    else
    {
        state.focused = false;

        ImGui::TextDisabled("Scene render target unavailable");
    }

    ImGui::End();   
}