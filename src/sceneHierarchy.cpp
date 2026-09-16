#include "sceneHierarchy.hpp"

#include <cstddef>

#include <imgui.h>

#include "editorSelection.hpp"
#include "object.hpp"
#include "scene.hpp"

namespace
{
    void DrawObjectNode(
        Object& object,
        EditorSelection& selection,
        ObjectId& pendingCreateChild,
        ObjectId& pendingDelete,
        ObjectId& pendingReparentObject,
        ObjectId& pendingReparentParent
    )
    {
        const bool hasChildren =
            object.getChildCount() > 0;

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        if (selection.isSelected(object.getId()))
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (!hasChildren)
        {
            flags |=
                ImGuiTreeNodeFlags_Leaf |
                ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        if (!object.isActive())
        {
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                ImGui::GetStyleColorVec4(
                    ImGuiCol_TextDisabled
                )
            );
        }

        const char* displayName =
            object.getName().empty()
                ? "<Unnamed>"
                : object.getName().c_str();

        const bool nodeOpen = ImGui::TreeNodeEx(
            static_cast<const void*>(&object),
            flags,
            "%s",
            displayName
        );

        if (!object.isActive())
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            selection.selectObject(&object);
        }

        if (ImGui::BeginDragDropSource())
        {
            const ObjectId draggedObjectId =
                object.getId();

            ImGui::SetDragDropPayload(
                "SCENE_OBJECT_ID",
                &draggedObjectId,
                sizeof(draggedObjectId)
            );

            ImGui::Text(
                "Move %s",
                displayName
            );

            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload(
                    "SCENE_OBJECT_ID"
                );

            if (
                payload != nullptr &&
                payload->DataSize ==
                    static_cast<int>(sizeof(ObjectId))
            )
            {
                const ObjectId draggedObjectId =
                    *static_cast<const ObjectId*>(
                        payload->Data
                    );

                if (draggedObjectId != object.getId())
                {
                    pendingReparentObject =
                        draggedObjectId;

                    pendingReparentParent =
                        object.getId();
                }
            }

            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            // Right-clicking also selects the object.
            selection.selectObject(&object);

            if (ImGui::MenuItem("Create Child"))
            {
                pendingCreateChild = object.getId();
            }

            if (ImGui::MenuItem("Delete"))
            {
                pendingDelete = object.getId();
            }

            ImGui::EndPopup();
        }

        if (hasChildren && nodeOpen)
        {
            for (
                std::size_t index = 0;
                index < object.getChildCount();
                ++index
            )
            {
                Object* child = object.getChild(index);

                if (child != nullptr)
                {
                    DrawObjectNode(
                        *child,
                        selection,
                        pendingCreateChild,
                        pendingDelete,
                        pendingReparentObject,
                        pendingReparentParent
                    );
                }
            }

            ImGui::TreePop();
        }
    }
}

bool DrawSceneHierarchy(
    Scene& scene,
    EditorSelection& selection
)
{
    bool hierarchyChanged = false;

    ObjectId pendingCreateChild = InvalidObjectId;
    ObjectId pendingDelete = InvalidObjectId;
    ObjectId pendingReparentObject = InvalidObjectId;
    ObjectId pendingReparentParent = InvalidObjectId;

    if (ImGui::Button("Create Empty"))
    {
        Object* object =
            scene.createObject("GameObject");

        if (object != nullptr)
        {
            selection.selectObject(object);
            hierarchyChanged = true;
        }
    }

    ImGui::SameLine();

    Object* selected =
        selection.getSelectedObject(scene);

    ImGui::BeginDisabled(selected == nullptr);

    if (ImGui::Button("Delete"))
    {
        pendingDelete = selection.getSelectedId();
    }

    ImGui::EndDisabled();
    ImGui::Separator();

    ImGui::TextDisabled(
        "Drop here to move an object to the scene root"
    );

    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(
                "SCENE_OBJECT_ID"
            );

        if (
            payload != nullptr &&
            payload->DataSize ==
                static_cast<int>(sizeof(ObjectId))
        )
        {
            pendingReparentObject =
                *static_cast<const ObjectId*>(
                    payload->Data
                );

            pendingReparentParent =
                InvalidObjectId;
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::Separator();

    for (
        std::size_t index = 0;
        index < scene.getRootObjectCount();
        ++index
    )
    {
        Object* rootObject =
            scene.getRootObject(index);

        if (rootObject != nullptr)
        {
            DrawObjectNode(
                *rootObject,
                selection,
                pendingCreateChild,
                pendingDelete,
                pendingReparentObject,
                pendingReparentParent
            );
        }
    }

    if (scene.getRootObjectCount() == 0)
    {
        ImGui::TextDisabled("The scene is empty.");
    }

    // Apply changes after traversal so the hierarchy's object
    // collection is not modified while it is being drawn.
    if (pendingCreateChild != InvalidObjectId)
    {
        Object* child = scene.createChildObject(
            pendingCreateChild,
            "GameObject"
        );

        if (child != nullptr)
        {
            selection.selectObject(child);
            hierarchyChanged = true;
        }
    }
    
    if (pendingReparentObject != InvalidObjectId)
    {
        if (scene.reparentObject(
            pendingReparentObject,
            pendingReparentParent,
            true))
        {
            selection.selectObject(pendingReparentObject);
            hierarchyChanged = true;
        }
    }

    if (pendingDelete != InvalidObjectId)
    {
        Object* objectToDelete =
            scene.findObject(pendingDelete);

        Object* selectedObject =
            selection.getSelectedObject(scene);

        bool deletesSelection = false;

        for (
            Object* current = selectedObject;
            current != nullptr;
            current = current->getParentObject()
        )
        {
            if (current == objectToDelete)
            {
                deletesSelection = true;
                break;
            }
        }

        if (deletesSelection)
        {
            selection.clear();
        }

        if (scene.destroyObject(pendingDelete))
        {
            hierarchyChanged = true;
        }
    }

    return hierarchyChanged;
}