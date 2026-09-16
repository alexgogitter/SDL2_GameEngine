#pragma once
#include "engineApi.hpp"
#include <glm/glm.hpp>
#include "layer.hpp"
enum class CameraProjection
{
    Orthographic,
    Perspective
};

/// Common camera interface consumed by renderer passes.
///
/// Camera2D and Camera3D provide different projections without coupling render
/// submissions to a specific scene dimensionality.
class Camera
{
private:
    LayerMask cullingMask = AllLayers;
public:
    virtual ~Camera() = default;

    /// @return Matrix transforming world coordinates into camera/view space.
    virtual glm::mat4 getViewMatrix() const = 0;

    /// @return Matrix transforming view coordinates into OpenGL clip space.
    virtual glm::mat4 getProjectionMatrix() const = 0;

    /// @return Whether this camera is orthographic or perspective.
    virtual CameraProjection getProjectionType() const = 0;

    /// Updates projection dimensions after window or render-target resizing.
    /// @param width Logical viewport width in pixels; clamped by implementations.
    /// @param height Logical viewport height in pixels; clamped by implementations.
    virtual void setViewportSize(float width, float height) = 0;

    /// @return Projection multiplied by view, suitable for shader upload.
    glm::mat4 getViewProjectionMatrix() const
    {
        return getProjectionMatrix() * getViewMatrix();
    }

    void setCullingMask(LayerMask mask)
    {
        cullingMask = mask;
    }

    LayerMask getCullingMask() const
    {
        return cullingMask;
    }

    bool setLayerVisible(int layer, bool visible)
    {
        if (!IsValidLayer(layer))
        {
            return false;
        }

        const LayerMask bit = LayerBit(layer);

        if (visible)
        {
            cullingMask |= bit;
        }
        else
        {
            cullingMask &= ~bit;
        }

        return true;
    }

    bool rendersLayer(int layer) const
    {
        return LayerMaskContains(cullingMask, layer);
    }

};
