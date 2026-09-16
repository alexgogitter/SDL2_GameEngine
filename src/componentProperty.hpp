#pragma once

#include <cstdint>

enum class ComponentPropertyType : std::uint8_t
{
    Boolean,
    Integer,
    Enumeration,
    Float,
    Vector2,
    Vector3,
    Vector4,
    Colour3,
    Colour4,
    String
};

struct ComponentProperty
{
    /// Stable key used by scene serialization.
    const char* key = "";

    /// Human-readable label shown by the editor.
    const char* displayName = "";

    ComponentPropertyType type =
        ComponentPropertyType::Float;

    /// Pointer to the actual component field.
    void* value = nullptr;

    bool readOnly = false;
    bool serializable = true;

    bool hasRange = false;
    float minimum = 0.0f;
    float maximum = 0.0f;
    float step = 0.1f;

    const char* const* enumerationLabels = nullptr;
    int enumerationCount = 0;
};