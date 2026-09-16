#include "editorSelection.hpp"

#include "object.hpp"
#include "scene.hpp"

void EditorSelection::selectObject(Object* object)
{
    selectedId =
        object != nullptr
            ? object->getId()
            : InvalidObjectId;
}

void EditorSelection::selectObject(ObjectId objectId)
{
    selectedId = objectId;
}

void EditorSelection::clear()
{
    selectedId = InvalidObjectId;
}

ObjectId EditorSelection::getSelectedId() const
{
    return selectedId;
}

bool EditorSelection::isSelected(ObjectId objectId) const
{
    return
        objectId != InvalidObjectId &&
        selectedId == objectId;
}

Object* EditorSelection::getSelectedObject(Scene& scene)
{
    if (selectedId == InvalidObjectId)
    {
        return nullptr;
    }

    Object* selected = scene.findObject(selectedId);

    if (selected == nullptr)
    {
        clear();
    }

    return selected;
}