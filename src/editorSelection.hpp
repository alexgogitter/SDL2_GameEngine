#pragma once

#include "engineApi.hpp"
#include "objectId.hpp"

class Object;
class Scene;

class EditorSelection
{
public:
    ENGINE_API void selectObject(Object* object);
    ENGINE_API void selectObject(ObjectId objectId);
    ENGINE_API void clear();

    ENGINE_API ObjectId getSelectedId() const;
    ENGINE_API bool isSelected(ObjectId objectId) const;

    /// Resolves the selection and clears it if the object was deleted.
    ENGINE_API Object* getSelectedObject(Scene& scene);

private:
    ObjectId selectedId = InvalidObjectId;
};