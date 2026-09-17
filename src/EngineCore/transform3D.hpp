#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/matrix_decompose.hpp>

#include "transform.hpp"

class Transform3D
{
  public:
    void setPosition(const glm::vec3 &value) { position = value; }

    // Compatibility for existing 2D components.
    void setPosition(const glm::vec2 &value)
    {
        position.x = value.x;
        position.y = value.y;
    }

    void setRotation(const glm::quat &value)
    {
        const float quaternionLength = glm::length(value);

        rotation = quaternionLength > 0.0f ? glm::normalize(value) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // Sets the Z rotation while preserving X and Y rotation.
    void setRotation(float zRadians)
    {
        glm::vec3 euler = getEulerRadians();
        euler.z = zRadians;
        setEulerRadians(euler);
    }

    void setEulerRadians(const glm::vec3 &value) { rotation = glm::normalize(glm::quat(value)); }

    void setScale(const glm::vec3 &value) { scale = value; }

    // Compatibility for existing 2D components.
    void setScale(const glm::vec2 &value)
    {
        scale.x = value.x;
        scale.y = value.y;
    }

    void translate(const glm::vec3 &delta) { position += delta; }

    void translate(const glm::vec2 &delta)
    {
        position.x += delta.x;
        position.y += delta.y;
    }

    void rotateBy(const glm::quat &delta) { rotation = glm::normalize(delta * rotation); }

    void rotateBy(float zRadians) { setRotation(getRotationZ() + zRadians); }

    void scaleBy(const glm::vec3 &factor) { scale *= factor; }

    void scaleBy(const glm::vec2 &factor)
    {
        scale.x *= factor.x;
        scale.y *= factor.y;
    }

    const glm::vec3 &getPosition() const { return position; }

    glm::vec2 getPosition2D() const { return {position.x, position.y}; }

    const glm::quat &getRotation() const { return rotation; }

    glm::vec3 getEulerRadians() const { return glm::eulerAngles(rotation); }

    float getRotationZ() const { return getEulerRadians().z; }

    const glm::vec3 &getScale() const { return scale; }

    glm::vec2 getScale2D() const { return {scale.x, scale.y}; }

    glm::mat4 getModelMatrix() const
    {
        glm::mat4 result(1.0f);

        result = glm::translate(result, position);
        result *= glm::mat4_cast(rotation);
        result = glm::scale(result, scale);

        return result;
    }

    Transform2D toTransform2D() const
    {
        Transform2D result;

        result.setPosition(getPosition2D());
        result.setRotation(getRotationZ());
        result.setScale(getScale2D());

        return result;
    }

    bool setFromMatrix(const glm::mat4 &matrix)
    {
        glm::vec3 decomposedScale;
        glm::quat decomposedRotation;
        glm::vec3 decomposedPosition;
        glm::vec3 skew;
        glm::vec4 perspective;

        if (!glm::decompose(matrix, decomposedScale, decomposedRotation, decomposedPosition, skew, perspective)) {
            return false;
        }

        if (glm::length(decomposedRotation) <= 0.0f) {
            return false;
        }

        position = decomposedPosition;
        scale = decomposedScale;
        rotation = glm::normalize(decomposedRotation);

        return true;
    }

  private:
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};
