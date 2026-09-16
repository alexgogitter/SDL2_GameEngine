#include "objectInspector.hpp"

#include "component.hpp"
#include "componentInspector.hpp"
#include "componentRegistry.hpp"
#include "layer.hpp"
#include "layerRegistry.hpp"
#include "object.hpp"

#include <imgui.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    std::string Lowercase(const char* text)
    {
        std::string result =
            text != nullptr ? text : "";

        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(
                    std::tolower(character)
                );
            }
        );

        return result;
    }

    bool MatchesComponentSearch(
        const char* typeName,
        const char* displayName,
        const char* category,
        const char* query
    )
    {
        if (query == nullptr || query[0] == '\0')
        {
            return true;
        }

        const std::string search =
            Lowercase(query);

        return
            Lowercase(typeName).find(search) !=
                std::string::npos ||
            Lowercase(displayName).find(search) !=
                std::string::npos ||
            Lowercase(category).find(search) !=
                std::string::npos;
    }

    bool DrawBurgerButton(const char* id)
    {
        const float size =
            ImGui::GetFrameHeight();

        const bool clicked =
            ImGui::InvisibleButton(
                id,
                ImVec2(size, size)
            );

        const bool hovered =
            ImGui::IsItemHovered();

        const bool active =
            ImGui::IsItemActive();

        const ImVec2 minimum =
            ImGui::GetItemRectMin();

        const ImVec2 maximum =
            ImGui::GetItemRectMax();

        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        const ImGuiCol backgroundColour =
            active
                ? ImGuiCol_ButtonActive
                : hovered
                    ? ImGuiCol_ButtonHovered
                    : ImGuiCol_Button;

        drawList->AddRectFilled(
            minimum,
            maximum,
            ImGui::GetColorU32(
                backgroundColour
            ),
            ImGui::GetStyle().FrameRounding
        );

        const float horizontalPadding =
            size * 0.27f;

        const float centreY =
            (minimum.y + maximum.y) * 0.5f;

        const float spacing =
            size * 0.18f;

        const ImU32 lineColour =
            ImGui::GetColorU32(
                ImGuiCol_Text
            );

        for (int line = -1; line <= 1; ++line)
        {
            const float y =
                centreY +
                static_cast<float>(line) *
                    spacing;

            drawList->AddLine(
                ImVec2(
                    minimum.x +
                        horizontalPadding,
                    y
                ),
                ImVec2(
                    maximum.x -
                        horizontalPadding,
                    y
                ),
                lineColour,
                1.5f
            );
        }

        if (hovered)
        {
            ImGui::SetTooltip(
                "Component options"
            );
        }

        return clicked;
    }
}

bool DrawObjectInspector(
    Object& object,
    LayerRegistry& layers,
    ComponentRegistry& componentRegistry,
    const ComponentCreateContext& componentContext,
    bool structuralEditingAllowed
)
{
    bool anythingChanged = false;

    ImGui::PushID(static_cast<const void*>(&object));

    std::array<char, 256> nameBuffer{};
    std::snprintf(
        nameBuffer.data(),
        nameBuffer.size(),
        "%s",
        object.getName().c_str()
    );

    if (ImGui::InputText(
        "Name",
        nameBuffer.data(),
        nameBuffer.size()))
    {
        object.setName(nameBuffer.data());
        anythingChanged = true;
    }

    ImGui::Text(
        "Object ID: %llu",
        static_cast<unsigned long long>(object.getId())
    );

    bool active = object.isActive();

    if (ImGui::Checkbox("Active", &active))
    {
        object.setActive(active);
        anythingChanged = true;
    }

    const int currentLayer = object.getLayer();
    const char* currentLayerName =
        layers.getLayerName(currentLayer).c_str();

    std::string currentLayerLabel =
        currentLayerName[0] != '\0'
            ? currentLayerName
            : "Layer " + std::to_string(currentLayer);

    if (ImGui::BeginCombo(
        "Layer",
        currentLayerLabel.c_str()))
    {
        for (int layer = 0; layer < LayerCount; ++layer)
        {
            const char* layerName =
                layers.getLayerName(layer).c_str();

            if (layerName[0] == '\0')
            {
                continue;
            }

            const bool selected = layer == currentLayer;

            if (ImGui::Selectable(layerName, selected))
            {
                object.setLayer(layer);
                anythingChanged = true;
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::CollapsingHeader(
        "Transform",
        ImGuiTreeNodeFlags_DefaultOpen))
    {

        Object* parent = object.getParentObject();

        if (parent != nullptr)
        {
            ImGui::TextDisabled(
                "Local transform relative to: %s",
                parent->getName().c_str()
            );

            const glm::vec3 worldPosition =
                object.getWorldPosition();

            ImGui::Text(
                "World Position: %.2f, %.2f, %.2f",
                worldPosition.x,
                worldPosition.y,
                worldPosition.z
            );

            ImGui::Separator();
        }
        else
        {
            ImGui::TextDisabled("Root object");
        }

        glm::vec3 position =
        object.transform.getPosition();

        if (ImGui::DragFloat3(
            "Position",
            &position.x,
            0.1f))
        {
            object.transform.setPosition(position);
            object.notifyTransformChanged();
            anythingChanged = true;
        }

        glm::vec3 rotationDegrees =
            glm::degrees(
                object.transform.getEulerRadians()
            );

        if (ImGui::DragFloat3(
            "Rotation",
            &rotationDegrees.x,
            0.5f))
        {
            object.transform.setEulerRadians(
                glm::radians(rotationDegrees)
            );

            object.notifyTransformChanged();
            anythingChanged = true;
        }

        glm::vec3 scale =
            object.transform.getScale();

        if (ImGui::DragFloat3(
            "Scale",
            &scale.x,
            0.05f))
        {
            object.transform.setScale(scale);
            object.notifyTransformChanged();
            anythingChanged = true;
        }
    }

    ImGui::SeparatorText("Components");

    std::string pendingRemovalType;

    for (
        std::size_t index = 0;
        index < object.getComponentCount();
        ++index
    )
    {
        Component* component =
            object.getComponentAt(index);

        if (component == nullptr)
        {
            continue;
        }

        ImGui::PushID(
            static_cast<const void*>(component)
        );

        const char* registeredDisplayName =
            componentRegistry.getDisplayName(
                component->getTypeName().c_str()
            );

        const char* displayName =
            registeredDisplayName != nullptr &&
            registeredDisplayName[0] != '\0'
                ? registeredDisplayName
                : component->getTypeName().c_str();

        std::string title = displayName;

        if (!component->isEnabled())
        {
            title += " (Disabled)";
        }

        bool componentOpen = false;
        bool componentMenuRequested = false;

        if (
            ImGui::BeginTable(
                "ComponentHeader",
                2,
                ImGuiTableFlags_SizingStretchProp
            )
        )
        {
            ImGui::TableSetupColumn(
                "Component",
                ImGuiTableColumnFlags_WidthStretch
            );

            ImGui::TableSetupColumn(
                "Options",
                ImGuiTableColumnFlags_WidthFixed,
                ImGui::GetFrameHeight()
            );

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);

            if (!component->isEnabled())
            {
                ImGui::PushStyleColor(
                    ImGuiCol_Text,
                    ImGui::GetStyleColorVec4(
                        ImGuiCol_TextDisabled
                    )
                );
            }

            const std::string headerLabel =
            title + "###ComponentCard";

            componentOpen = ImGui::CollapsingHeader(headerLabel.c_str(),ImGuiTreeNodeFlags_DefaultOpen);
            if (!component->isEnabled())
            {
                ImGui::PopStyleColor();
            }

            ImGui::TableSetColumnIndex(1);
            componentMenuRequested = DrawBurgerButton("##ComponentMenuButton");

            ImGui::EndTable();
        }

        if (componentMenuRequested)
        {
            ImGui::OpenPopup("ComponentMenu");
        }

        if (ImGui::BeginPopup("ComponentMenu"))
        {
            if (
                ImGui::MenuItem(
                    component->isEnabled()
                        ? "Disable Component"
                        : "Enable Component"
                )
            )
            {
                component->setEnabled(
                    !component->isEnabled()
                );

                anythingChanged = true;
            }

            ImGui::Separator();

            const char* removalBlocker =
                componentRegistry.getRemovalBlocker(
                    component->getTypeName().c_str(),
                    object
                );

            const bool removalBlocked =
                removalBlocker != nullptr &&
                removalBlocker[0] != '\0';

            if (!structuralEditingAllowed)
            {
                ImGui::TextDisabled(
                    "Stop Play Mode to remove components."
                );
            }
            else if (removalBlocked)
            {
                const char* blockerDisplayName =
                    componentRegistry.getDisplayName(
                        removalBlocker
                    );

                ImGui::TextDisabled(
                    "Required by %s",
                    blockerDisplayName != nullptr &&
                    blockerDisplayName[0] != '\0'
                        ? blockerDisplayName
                        : removalBlocker
                );
            }

            ImGui::BeginDisabled(
                !structuralEditingAllowed ||
                removalBlocked
            );

            if (ImGui::MenuItem("Remove Component"))
            {
                pendingRemovalType =
                    component->getTypeName();
            }

            ImGui::EndDisabled();
            ImGui::EndPopup();
        }

        if (componentOpen)
        {
            if (component->getPropertyCount() == 0)
            {
                ImGui::TextDisabled(
                    "No editable properties"
                );
            }
            else
            {
                anythingChanged |=
                    DrawComponentInspector(
                        *component
                    );
            }
        }

        ImGui::PopID();
    }

    if (!pendingRemovalType.empty())
    {
        if (
            componentRegistry.removeFromObject(
                pendingRemovalType.c_str(),
                object
            )
        )
        {
            anythingChanged = true;
        }
    }

    if (object.getComponentCount() == 0)
    {
        ImGui::TextDisabled(
            "No components attached"
        );
    }

    ImGui::Spacing();

    static std::array<char, 128>
        componentSearch{};

    static std::string
        componentAdditionMessage;

    static bool componentAdditionFailed = false;

    std::string pendingAdditionType;

    ImGui::BeginDisabled(
        !structuralEditingAllowed
    );

    if (
        ImGui::Button(
            "Add Component",
            ImVec2(
                ImGui::GetContentRegionAvail().x,
                0.0f
            )
        )
    )
    {
        ImGui::OpenPopup(
            "AddComponentPopup"
        );
    }

    ImGui::EndDisabled();

    if (!structuralEditingAllowed)
    {
        ImGui::TextDisabled(
            "Stop Play Mode to add or remove components."
        );
    }

    ImGui::SetNextWindowSize(
        ImVec2(360.0f, 420.0f),
        ImGuiCond_Appearing
    );

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
    if (ImGui::IsWindowAppearing())
    {
        componentSearch.fill('\0');
        componentAdditionMessage.clear();

        ImGui::SetKeyboardFocusHere();
    }

    ImGui::InputTextWithHint(
        "##ComponentSearch",
        "Search components...",
        componentSearch.data(),
        componentSearch.size()
    );

    ImGui::Separator();

    std::vector<std::string> categories;

    for (
        std::size_t typeIndex = 0;
        typeIndex <
            componentRegistry.getTypeCount();
        ++typeIndex
    )
    {
        const char* typeName =
            componentRegistry.getTypeNameAt(
                typeIndex
            );

        if (
            typeName == nullptr ||
            typeName[0] == '\0' ||
            object.getComponentByTypeName(
                typeName
            ) != nullptr
        )
        {
            continue;
        }

        const char* displayName =
            componentRegistry.getDisplayName(
                typeName
            );

        const char* category =
            componentRegistry.getCategory(
                typeName
            );

        if (
            !MatchesComponentSearch(
                typeName,
                displayName,
                category,
                componentSearch.data()
            )
        )
        {
            continue;
        }

        const std::string categoryName =
            category != nullptr &&
            category[0] != '\0'
                ? category
                : "Other";

        if (
            std::find(
                categories.begin(),
                categories.end(),
                categoryName
            ) == categories.end()
        )
        {
            categories.push_back(
                categoryName
            );
        }
    }

    std::sort(
        categories.begin(),
        categories.end()
    );

    if (categories.empty())
    {
        ImGui::TextDisabled(
            "No matching components"
        );
    }

    for (const std::string& category : categories)
    {
        ImGui::SeparatorText(category.c_str());

            for (std::size_t typeIndex = 0;
                typeIndex < componentRegistry.getTypeCount();
                ++typeIndex
            )
            {
                const char* typeName = componentRegistry.getTypeNameAt(typeIndex);
                if (typeName == nullptr ||
                    typeName[0] == '\0' ||
                    object.getComponentByTypeName(typeName) != nullptr)
                {
                    continue;
                }

                const char* displayName =
                    componentRegistry.getDisplayName(
                        typeName
                    );

                const char* typeCategory =
                    componentRegistry.getCategory(
                        typeName
                    );

                const std::string categoryName =
                    typeCategory != nullptr &&
                    typeCategory[0] != '\0'
                        ? typeCategory
                        : "Other";

                if (
                    categoryName != category ||
                    !MatchesComponentSearch(
                        typeName,
                        displayName,
                        typeCategory,
                        componentSearch.data()
                    )
                )
                {
                    continue;
                }

                ImGui::PushID(typeName);

                const char* itemLabel =
                    displayName != nullptr &&
                    displayName[0] != '\0'
                        ? displayName
                        : typeName;

                if (ImGui::Selectable(itemLabel))
                {
                    pendingAdditionType = typeName;

                    ImGui::CloseCurrentPopup();
                }

                const std::size_t requirementCount = componentRegistry.getRequiredTypeCount(typeName);

                if (
                    requirementCount > 0 &&
                    ImGui::IsItemHovered()
                )
                {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(
                        "Automatically adds:"
                    );

                    for (
                        std::size_t requirement = 0;
                        requirement < requirementCount;
                        ++requirement
                    )
                    {
                        const char* requiredType =
                            componentRegistry.
                                getRequiredTypeNameAt(
                                    typeName,
                                    requirement
                                );

                        const char* requiredDisplayName =
                            componentRegistry.
                                getDisplayName(
                                    requiredType
                                );

                        ImGui::BulletText(
                            "%s",
                            requiredDisplayName != nullptr &&
                            requiredDisplayName[0] != '\0'
                                ? requiredDisplayName
                                : requiredType
                        );
                    }

                    ImGui::EndTooltip();
                }

                ImGui::PopID();
            }
        }

        ImGui::EndPopup();
    }


    
    if (!pendingAdditionType.empty())
    {
        Component* addedComponent = componentRegistry.createAndAttach(pendingAdditionType.c_str(), object, componentContext);

        printf("Added component: %s\n", pendingAdditionType.c_str());

        if (addedComponent != nullptr)
        {
            const char* displayName = componentRegistry.getDisplayName(pendingAdditionType.c_str());

            componentAdditionMessage = "Added " + std::string(displayName != nullptr && displayName[0] != '\0' ? displayName : pendingAdditionType);

            componentAdditionFailed = false;
            anythingChanged = true;
        }
        else
        {
            componentAdditionMessage = "Unable to create " + pendingAdditionType;
            componentAdditionFailed = true;
        }
    }

    if (!componentAdditionMessage.empty())
    {
        const ImVec4 messageColour =
            componentAdditionFailed
                ? ImVec4(
                    1.0f,
                    0.35f,
                    0.35f,
                    1.0f
                )
                : ImVec4(
                    0.35f,
                    1.0f,
                    0.45f,
                    1.0f
                );

        ImGui::TextColored(
            messageColour,
            "%s",
            componentAdditionMessage.c_str()
        );
    }

    ImGui::PopID();
    return anythingChanged;
}