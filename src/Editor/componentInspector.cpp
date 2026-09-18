#include "componentInspector.hpp"

#include "component.hpp"
#include "componentProperty.hpp"

#include <glm.hpp>
#include <imgui.h>

#include <string>
#include <vector>
#include <cstring>

namespace
{
bool DrawProperty(ComponentProperty &property)
{
    const char *label = property.displayName != nullptr && property.displayName[0] != '\0' ? property.displayName : property.key;

    const float minimum = property.hasRange ? property.minimum : 0.0f;
    const float maximum = property.hasRange ? property.maximum : 0.0f;

    const ImGuiSliderFlags flags = property.hasRange ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;

    bool changed = false;

    if (property.readOnly) {
        ImGui::BeginDisabled();
    }

    switch (property.type) {
    case ComponentPropertyType::Boolean:
        changed = ImGui::Checkbox(label, static_cast<bool *>(property.value));
        break;

    case ComponentPropertyType::Integer: {
        auto *value = static_cast<int *>(property.value);

        if (property.hasRange) {
            changed = ImGui::SliderInt(label, value, static_cast<int>(property.minimum), static_cast<int>(property.maximum));
        }
        else {
            changed = ImGui::DragInt(label, value, property.step);
        }
        break;
    }

    case ComponentPropertyType::Enumeration: {
        auto *value = static_cast<int *>(property.value);

        const bool validSelection = value != nullptr && property.enumerationLabels != nullptr && *value >= 0 && *value < property.enumerationCount;

        const char *preview = validSelection ? property.enumerationLabels[*value] : "<Invalid>";

        if (ImGui::BeginCombo(label, preview)) {
            for (int index = 0; index < property.enumerationCount; ++index) {
                const bool selected = value != nullptr && *value == index;

                if (ImGui::Selectable(property.enumerationLabels[index], selected)) {
                    *value = index;
                    changed = true;
                }

                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        break;
    }

    case ComponentPropertyType::Float:
        changed = ImGui::DragFloat(label, static_cast<float *>(property.value), property.step, minimum, maximum, "%.3f", flags);
        break;

    case ComponentPropertyType::Vector2: {
        auto *value = static_cast<glm::vec2 *>(property.value);

        changed = ImGui::DragFloat2(label, &value->x, property.step, minimum, maximum, "%.3f", flags);
        break;
    }

    case ComponentPropertyType::Vector3: {
        auto *value = static_cast<glm::vec3 *>(property.value);

        changed = ImGui::DragFloat3(label, &value->x, property.step, minimum, maximum, "%.3f", flags);
        break;
    }

    case ComponentPropertyType::Vector4: {
        auto *value = static_cast<glm::vec4 *>(property.value);

        changed = ImGui::DragFloat4(label, &value->x, property.step, minimum, maximum, "%.3f", flags);
        break;
    }

    case ComponentPropertyType::Colour3: {
        auto *value = static_cast<glm::vec3 *>(property.value);

        changed = ImGui::ColorEdit3(label, &value->x);
        break;
    }

    case ComponentPropertyType::Colour4: {
        auto *value = static_cast<glm::vec4 *>(property.value);

        changed = ImGui::ColorEdit4(label, &value->x);
        break;
    }

    case ComponentPropertyType::String: {
        auto *value = static_cast<std::string *>(property.value);
        std::vector<char> buffer((std::max)(std::size_t(4096), value->size() + 1024), 0);
        std::memcpy(buffer.data(), value->data(), value->size());
        if (ImGui::InputText(label, buffer.data(), buffer.size())) { *value = buffer.data(); changed = true; }
        if (!property.readOnly && ImGui::BeginDragDropTarget()) {
            if (const auto *payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                *value = static_cast<const char *>(payload->Data); changed = true;
            }
            ImGui::EndDragDropTarget();
        }
        break;
    }
    }

    if (property.readOnly) {
        ImGui::EndDisabled();
    }

    return changed;
}
} // namespace

bool DrawComponentInspector(Component &component)
{
    bool anythingChanged = false;

    ImGui::PushID(static_cast<const void *>(&component));

    for (std::size_t index = 0; index < component.getPropertyCount(); ++index) {
        ComponentProperty property;

        if (!component.getProperty(index, property)) {
            continue;
        }

        ImGui::PushID(property.key);

        if (DrawProperty(property)) {
            component.notifyPropertyChanged(property.key);
            anythingChanged = true;
        }

        ImGui::PopID();
    }

    ImGui::PopID();
    return anythingChanged;
}