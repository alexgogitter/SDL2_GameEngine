#pragma once

#include <limits>

#include "component.hpp"
#include "resource_manager.hpp"

// Renders a full texture over its parent Object's quad. Position, size and
// rotation come from the same Transform2D used by physics components.
class TextureComponent final : public Component
{
public:
    TextureComponent(
        Object* parent,
        Resource_manager& resources,
        const char* texturePath = nullptr
    );

    bool setTexture(const char* texturePath);
    void clearTexture();

    bool hasTexture() const { return textureId != InvalidTextureId; }
    unsigned int getTextureId() const { return textureId; }

private:
    void Update(std::uint64_t deltaTime) override;
    void Draw(Renderer* renderer) override;

    static constexpr unsigned int InvalidTextureId =
        std::numeric_limits<unsigned int>::max();

    Resource_manager* resources = nullptr;
    unsigned int textureId = InvalidTextureId;
};
