#include "render2D.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>

#include <SDL_image.h>
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "shader.hpp"
#include "window.hpp"

namespace
{
constexpr int MaximumPointLights2D = 16;

glm::mat4 buildModelMatrix(const Transform2D &transform)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(transform.getPosition(), 0.0f));
    model = glm::rotate(model, transform.getRotation(), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(transform.getScale() * 2.0f, 1.0f));
    return model;
}

unsigned int uploadSurfaceTexture(SDL_Surface *sourceSurface)
{
    if (sourceSurface == nullptr) {
        return 0;
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(sourceSurface, SDL_PIXELFORMAT_RGBA32, 0);
    if (surface == nullptr) {
        return 0;
    }

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surface);
    return texture;
}
} // namespace

Renderer2D::Renderer2D(Window &ownerWindow) : window(&ownerWindow) {}

Renderer2D::~Renderer2D() { Renderer_Close(); }

void Renderer2D::BeginFrame()
{
    renderCommands2D.clear();
    lights2D.clear();
    nextSubmissionOrder = 0;
}

int Renderer2D::Renderer_Init() { return initialise2DRenderer() ? 0 : 1; }

bool Renderer2D::initialise2DRenderer()
{
    spriteShader = std::make_unique<Shader>();
    if (!spriteShader->loadFromFiles("res/shaders/sprite2d.vert", "res/shaders/sprite2d.frag")) {
        return false;
    }

    const float vertices[] = {-0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.0f, 1.0f};
    const unsigned int indices[] = {0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &quadVertexArray);
    glGenBuffers(1, &quadVertexBuffer);
    glGenBuffers(1, &quadIndexBuffer);

    glBindVertexArray(quadVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    glBindVertexArray(0);

    spriteShader->use();
    spriteShader->setInt("uAlbedoTexture", 0);
    spriteShader->setInt("uNormalTexture", 1);
    spriteShader->setInt("uHeightTexture", 2);
    spriteShader->setInt("uEmissionTexture", 3);
    spriteShader->setInt("uDiffuseTexture", 4);
    spriteShader->setInt("uSpecularTexture", 5);
    spriteShader->setInt("uAlphaMaskTexture", 6);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}

void Renderer2D::Renderer_Close() { destroy2DRenderer(); }

void Renderer2D::destroy2DRenderer()
{
    for (const RenderCommand2D &command : renderCommands2D) {
        if (command.deleteAlbedoAfterDraw && command.material.albedoTexture != 0) {
            glDeleteTextures(1, &command.material.albedoTexture);
        }
    }
    renderCommands2D.clear();

    if (quadIndexBuffer != 0) {
        glDeleteBuffers(1, &quadIndexBuffer);
        quadIndexBuffer = 0;
    }
    if (quadVertexBuffer != 0) {
        glDeleteBuffers(1, &quadVertexBuffer);
        quadVertexBuffer = 0;
    }
    if (quadVertexArray != 0) {
        glDeleteVertexArrays(1, &quadVertexArray);
        quadVertexArray = 0;
    }

    spriteShader.reset();
}

void Renderer2D::SetViewportSize(float width, float height)
{
    if (activeCamera != nullptr) {
        activeCamera->setViewportSize(width, height);
    }
}
bool Renderer2D::shouldRenderLayer(int objectLayer) const
{
    if (!IsValidLayer(objectLayer)) {
        return false;
    }

    return activeCamera == nullptr || activeCamera->rendersLayer(objectLayer);
}

void Renderer2D::SubmitSolidQuad2D(const Transform2D &transform, const glm::vec4 &colour, int renderLayer, int objectLayer)
{
    if (!shouldRenderLayer(objectLayer)) {
        return;
    }
    Material2DRenderState material;
    material.tint = colour;
    material.lit = true;
    material.renderLayer = renderLayer;

    renderCommands2D.push_back({CommandType2D::Quad, transform, material, nextSubmissionOrder++, false});
}

void Renderer2D::SubmitSprite2D(const Transform2D &transform, const Material2DRenderState &material, int objectLayer)
{
    if (!shouldRenderLayer(objectLayer)) {
        return;
    }
    renderCommands2D.push_back({CommandType2D::Quad, transform, material, nextSubmissionOrder++, false});
}

void Renderer2D::SubmitOutline2D(const Transform2D &transform, const glm::vec4 &colour, int renderLayer, int objectLayer)
{
    if (!shouldRenderLayer(objectLayer)) {
        return;
    }
    Material2DRenderState material;
    material.tint = colour;
    material.lit = false;
    material.renderLayer = renderLayer;

    renderCommands2D.push_back({CommandType2D::Outline, transform, material, nextSubmissionOrder++, false});
}

void Renderer2D::SubmitLight2D(const PointLight2D &light, int objectLayer)
{
    if (!shouldRenderLayer(objectLayer)) {
        return;
    }

    if (lights2D.size() < MaximumPointLights2D) {
        lights2D.push_back(light);
    }
}

void Renderer2D::Render2D()
{
    if (spriteShader == nullptr || !spriteShader->isValid()) {
        return;
    }

    std::stable_sort(renderCommands2D.begin(), renderCommands2D.end(), [](const RenderCommand2D &left, const RenderCommand2D &right) {
        if (left.material.renderLayer == right.material.renderLayer) {
            return left.submissionOrder < right.submissionOrder;
        }
        return left.material.renderLayer < right.material.renderLayer;
    });

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    spriteShader->use();
    const glm::mat4 viewProjection = activeCamera != nullptr ? activeCamera->getViewProjectionMatrix() : glm::ortho(0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()), 0.0f, -100.0f, 100.0f);
    spriteShader->setMat4("uViewProjection", viewProjection);
    uploadLights2D();

    glBindVertexArray(quadVertexArray);
    for (const RenderCommand2D &command : renderCommands2D) {
        drawCommand2D(command);
        if (command.deleteAlbedoAfterDraw && command.material.albedoTexture != 0) {
            glDeleteTextures(1, &command.material.albedoTexture);
        }
    }
    glBindVertexArray(0);

    renderCommands2D.clear();
}

void Renderer2D::uploadLights2D()
{
    spriteShader->setInt("uLightCount", static_cast<int>(lights2D.size()));
    spriteShader->setVec3("uAmbientLight", glm::vec3(0.32f));

    for (std::size_t index = 0; index < lights2D.size(); ++index) {
        const std::string prefix = "uLights[" + std::to_string(index) + "].";
        spriteShader->setVec3(prefix + "position", lights2D[index].position);
        spriteShader->setVec3(prefix + "colour", lights2D[index].colour);
        spriteShader->setFloat(prefix + "intensity", lights2D[index].intensity);
        spriteShader->setFloat(prefix + "radius", lights2D[index].radius);
    }
}

void Renderer2D::drawCommand2D(const RenderCommand2D &command)
{
    spriteShader->setMat4("uModel", buildModelMatrix(command.transform));
    spriteShader->setVec4("uTint", command.material.tint);
    spriteShader->setFloat("uSpriteRotation", command.transform.getRotation());
    spriteShader->setFloat("uNormalStrength", command.material.normalStrength);
    spriteShader->setFloat("uHeightScale", command.material.heightScale);
    spriteShader->setFloat("uSpecularStrength", command.material.specularStrength);
    spriteShader->setFloat("uShininess", command.material.shininess);
    spriteShader->setVec4("uAlbedoUvRect", command.material.albedoUvRect);
    spriteShader->setVec4("uAlphaMaskUvRect", command.material.alphaMaskUvRect);
    spriteShader->setVec4("uDiffuseUvRect", command.material.diffuseUvRect);
    spriteShader->setVec4("uNormalUvRect", command.material.normalUvRect);
    spriteShader->setVec4("uHeightUvRect", command.material.heightUvRect);
    spriteShader->setVec4("uEmissionUvRect", command.material.emissionUvRect);
    spriteShader->setVec4("uSpecularUvRect", command.material.specularUvRect);
    spriteShader->setBool("uLit", command.material.lit);
    spriteShader->setBool("uHasAlbedo", command.material.albedoTexture != 0);
    spriteShader->setBool("uHasAlphaMask", command.material.alphaMaskTexture != 0);
    spriteShader->setBool("uHasDiffuse", command.material.diffuseTexture != 0);
    spriteShader->setBool("uHasNormal", command.material.normalTexture != 0);
    spriteShader->setBool("uHasHeight", command.material.heightTexture != 0);
    spriteShader->setBool("uHasEmission", command.material.emissionTexture != 0);
    spriteShader->setBool("uHasSpecular", command.material.specularTexture != 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, command.material.albedoTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, command.material.normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, command.material.heightTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, command.material.emissionTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, command.material.diffuseTexture);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, command.material.specularTexture);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, command.material.alphaMaskTexture);

    if (command.type == CommandType2D::Outline) {
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    else {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

void Renderer2D::Renderer_ttf(const std::string &text, TTF_Font *font, SDL_Rect *source, SDL_Rect *destination, SDL_Color &textColour)
{
    (void)source;
    if (font == nullptr || text.empty()) {
        return;
    }

    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColour);
    if (textSurface == nullptr) {
        return;
    }

    const unsigned int textTexture = uploadSurfaceTexture(textSurface);
    const int width = destination != nullptr ? destination->w : textSurface->w;
    const int height = destination != nullptr ? destination->h : textSurface->h;
    const int x = destination != nullptr ? destination->x : 0;
    const int y = destination != nullptr ? destination->y : 0;
    SDL_FreeSurface(textSurface);

    if (textTexture == 0) {
        return;
    }

    Transform2D transform;
    transform.setPosition({x + width * 0.5f, y + height * 0.5f});
    transform.setScale({width * 0.5f, height * 0.5f});

    Material2DRenderState material;
    material.albedoTexture = textTexture;
    material.lit = false;
    material.renderLayer = 10000;

    renderCommands2D.push_back({CommandType2D::Quad, transform, material, nextSubmissionOrder++, true});
}
