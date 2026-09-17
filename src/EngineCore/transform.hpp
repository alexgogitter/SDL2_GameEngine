#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include "glm.hpp"

/// Pixel-space 2D transform shared by Box2D synchronization and render commands.
/// Scale is a renderer half-size for submitted quads, but SpriteRendererComponent
/// owns a separate draw rect so sprite visuals do not need to match colliders.
class Transform2D
{
  private:
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

  public:
    /// Creates a transform at the origin with half-size (1,1) and zero rotation.
    Transform2D() : position(0.0f, 0.0f), scale(1.0f, 1.0f), rotation(0.0f) {}

    /// Sets world-space centre in pixels.
    void setPosition(const glm::vec2 &pos) { position = pos; }
    /// Sets submitted-quad half-size in pixels.
    void setScale(const glm::vec2 &scl) { scale = scl; }
    /// Sets rotation in radians.
    void setRotation(float rot) { rotation = rot; }

    /// Adds a world-space pixel offset.
    void translate(const glm::vec2 &delta) { position += delta; }
    /// Multiplies the current submitted-quad half-size component-wise.
    void scaleBy(const glm::vec2 &factor) { scale *= factor; }
    /// Adds a rotation in radians.
    void rotateBy(float delta) { rotation += delta; }

    /// @return World-space centre in pixels.
    const glm::vec2 &getPosition() const { return position; }
    /// @return Submitted-quad half-size in pixels.
    const glm::vec2 &getScale() const { return scale; }
    /// @return Rotation in radians.
    float getRotation() const { return rotation; }
};

#endif
