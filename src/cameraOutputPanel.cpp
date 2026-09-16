#include "cameraOutputPanel.hpp"

#include <cstddef>

#include <imgui.h>

#include "cameraComponent.hpp"
#include "cameraSystem.hpp"
#include "object.hpp"
#include "scene.hpp"

namespace
{
    void DrawOutputSelector(
        const char* label,
        CameraOutputTarget output,
        CameraSystem& cameras,
        Scene& scene
    )
    {
        CameraComponent* current =
            cameras.resolvePrimaryCamera(
                scene,
                output
            );

        Object* currentObject =
            current != nullptr
                ? current->getParent()
                : nullptr;

        const char* preview =
            currentObject != nullptr
                ? currentObject->getName().c_str()
                : "<No Camera>";

        if (ImGui::BeginCombo(label, preview))
        {
            const std::size_t cameraCount =
                cameras.getCameraCount(
                    scene,
                    output
                );

            if (cameraCount == 0)
            {
                ImGui::TextDisabled(
                    "No eligible cameras"
                );
            }

            for (
                std::size_t index = 0;
                index < cameraCount;
                ++index
            )
            {
                CameraComponent* candidate =
                    cameras.getCameraAt(
                        scene,
                        output,
                        index
                    );

                if (candidate == nullptr)
                {
                    continue;
                }

                Object* owner =
                    candidate->getParent();

                if (owner == nullptr)
                {
                    continue;
                }

                ImGui::PushID(
                    static_cast<const void*>(
                        candidate
                    )
                );

                const bool selected =
                    candidate == current;

                if (ImGui::Selectable(
                    owner->getName().c_str(),
                    selected))
                {
                    cameras.setPrimaryCamera(
                        scene,
                        output,
                        owner->getId()
                    );
                }

                ImGui::SameLine();

                ImGui::TextDisabled(
                    "Priority %d",
                    candidate->getPriority()
                );

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }
}

void DrawCameraOutputPanel(
    CameraSystem& cameras,
    Scene& scene
)
{
    ImGui::Begin("Camera Outputs");

    DrawOutputSelector(
        "Game View",
        CameraOutputTarget::GameView,
        cameras,
        scene
    );

    DrawOutputSelector(
        "Editor Preview",
        CameraOutputTarget::EditorPreview,
        cameras,
        scene
    );

    ImGui::Separator();

    ImGui::TextDisabled(
        "Render Texture cameras will be routed "
        "when framebuffer assets are added."
    );

    ImGui::TextDisabled(
        "The legacy prototype still uses Camera2D "
        "until the 3D render pass is connected."
    );

    ImGui::End();
}