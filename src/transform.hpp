#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include "glm/glm.hpp"


class Transform2D
{
    private:
        glm::vec2 position;
        glm::vec2 scale;
        float rotation;

    public:
        Transform2D() : position(0.0f, 0.0f), scale(1.0f, 1.0f), rotation(0.0f) {}

        void setPosition(const glm::vec2& pos) { position = pos; }
        void setScale(const glm::vec2& scl) { scale = scl; }
        void setRotation(float rot) { rotation = rot; }

        void translate(const glm::vec2& delta) { position += delta; }
        void scaleBy(const glm::vec2& factor) { scale *= factor; }
        void rotateBy(float delta) { rotation += delta; }

        const glm::vec2& getPosition() const { return position; }
        const glm::vec2& getScale() const { return scale; }
        float getRotation() const { return rotation; }
};

#endif
