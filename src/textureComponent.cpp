#include "textureComponent.hpp"

#include <cmath>

#include "object.hpp"
#include "render.hpp"

TextureComponent::TextureComponent(
    Object* parentObject,
    Resource_manager& resourceManager,
    const char* texturePath)
    : Component("TextureRenderer2D", parentObject),
      resources(&resourceManager)
{
    if (texturePath != nullptr)
    {
        setTexture(texturePath);
    }
}

bool TextureComponent::setTexture(const char* texturePath)
{
    if (resources == nullptr || texturePath == nullptr)
    {
        return false;
    }

    const unsigned int loadedTextureId = resources->loadTexture(texturePath);
    if (loadedTextureId == InvalidTextureId)
    {
        return false;
    }

    textureId = loadedTextureId;
    return true;
}

void TextureComponent::clearTexture()
{
    textureId = InvalidTextureId;
}

void TextureComponent::Update(std::uint64_t)
{
    // The RigidbodyComponent updates the shared parent transform.
}

void TextureComponent::Draw(Renderer* renderer)
{
    if (renderer == nullptr || parent == nullptr || !hasTexture())
    {
        return;
    }

    SDL_Texture* texture = resources->getTexture(textureId);
    if (texture == nullptr)
    {
        return;
    }

    const glm::vec2 centre = parent->transform.getPosition();
    const glm::vec2 halfSize = parent->transform.getScale();
    const float angle = parent->transform.getRotation();
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);

    auto rotatePoint = [&](float localX, float localY) -> SDL_FPoint
    {
        return {
            centre.x + (localX * cosine - localY * sine),
            centre.y + (localX * sine + localY * cosine)
        };
    };

    const SDL_Color white = {255, 255, 255, 255};
    SDL_Vertex vertices[4] = {
        { rotatePoint(-halfSize.x, -halfSize.y), white, {0.0f, 0.0f} },
        { rotatePoint( halfSize.x, -halfSize.y), white, {1.0f, 0.0f} },
        { rotatePoint( halfSize.x,  halfSize.y), white, {1.0f, 1.0f} },
        { rotatePoint(-halfSize.x,  halfSize.y), white, {0.0f, 1.0f} }
    };

    const int indices[6] = {0, 1, 2, 0, 2, 3};

    SDL_RenderGeometry(
        renderer->get_SDLRenderer(),
        texture,
        vertices,
        4,
        indices,
        6
    );
}
