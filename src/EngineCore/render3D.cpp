#include "render3D.hpp"

#include <algorithm>

#include <glad/glad.h>

#include "camera.hpp"
#include "shader.hpp"
#include "window.hpp"

Renderer3D::Renderer3D(Window &ownerWindow) : window(&ownerWindow) {}

Renderer3D::~Renderer3D() { Shutdown(); }

bool Renderer3D::Initialize()
{
    if (meshShader != nullptr && meshOutlineShader != nullptr) {
        return meshShader->isValid() && meshOutlineShader->isValid();
    }

    meshShader = std::make_unique<Shader>();
    meshOutlineShader = std::make_unique<Shader>();

    const bool meshLoaded = meshShader->loadFromFiles("res/shaders/mesh3d.vert", "res/shaders/mesh3d.frag");
    const bool outlineLoaded = meshOutlineShader->loadFromFiles("res/shaders/mesh3d_outline.vert", "res/shaders/mesh3d_outline.frag");

    return meshLoaded && outlineLoaded;
}

void Renderer3D::Shutdown()
{
    renderCommands.clear();
    selectionOutlineCommands.clear();
    meshShader.reset();
    meshOutlineShader.reset();
}

void Renderer3D::BeginFrame()
{
    renderCommands.clear();
    selectionOutlineCommands.clear();
}

void Renderer3D::SetViewportSize(float width, float height)
{
    if (activeCamera != nullptr) {
        activeCamera->setViewportSize(width, height);
    }
}

bool Renderer3D::shouldRenderLayer(int objectLayer) const { return IsValidLayer(objectLayer) && (activeCamera == nullptr || activeCamera->rendersLayer(objectLayer)); }

void Renderer3D::SubmitMesh(const MeshRenderState &mesh, const glm::mat4 &worldMatrix, const glm::vec4 &colour, int objectLayer)
{
    if (mesh.vertexArray == 0 || mesh.indexCount == 0 || !shouldRenderLayer(objectLayer)) {
        return;
    }

    renderCommands.push_back({mesh.vertexArray, mesh.indexCount, worldMatrix, colour});
}

void Renderer3D::SubmitSelectionOutline(const MeshRenderState &mesh, const glm::mat4 &worldMatrix, const glm::vec4 &colour, float shellScale, int objectLayer)
{
    if (mesh.vertexArray == 0 || mesh.indexCount == 0 || !shouldRenderLayer(objectLayer)) {
        return;
    }

    selectionOutlineCommands.push_back({mesh.vertexArray, mesh.indexCount, worldMatrix, colour, std::max(shellScale, 1.001f)});
}

void Renderer3D::Render3D()
{
    if (meshShader == nullptr || !meshShader->isValid() || activeCamera == nullptr) {
        renderCommands.clear();
        selectionOutlineCommands.clear();
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_BLEND);

    meshShader->use();
    meshShader->setMat4("uViewProjection", activeCamera->getViewProjectionMatrix());

    for (const RenderCommand &command : renderCommands) {
        meshShader->setMat4("uModel", command.worldMatrix);
        meshShader->setVec4("uColour", command.colour);
        glBindVertexArray(command.vertexArray);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.indexCount), GL_UNSIGNED_INT, nullptr);
    }

    if (!selectionOutlineCommands.empty() && meshOutlineShader != nullptr && meshOutlineShader->isValid()) {
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        meshShader->use();
        meshShader->setMat4("uViewProjection", activeCamera->getViewProjectionMatrix());
        for (const SelectionOutlineCommand &command : selectionOutlineCommands) {
            meshShader->setMat4("uModel", command.worldMatrix);
            glBindVertexArray(command.vertexArray);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.indexCount), GL_UNSIGNED_INT, nullptr);
        }

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glCullFace(GL_FRONT);

        meshOutlineShader->use();
        meshOutlineShader->setMat4("uViewProjection", activeCamera->getViewProjectionMatrix());
        for (const SelectionOutlineCommand &command : selectionOutlineCommands) {
            meshOutlineShader->setMat4("uModel", command.worldMatrix);
            meshOutlineShader->setVec4("uOutlineColour", command.colour);
            meshOutlineShader->setFloat("uOutlineScale", command.shellScale);
            glBindVertexArray(command.vertexArray);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.indexCount), GL_UNSIGNED_INT, nullptr);
        }

        glCullFace(GL_BACK);
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
    }

    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    renderCommands.clear();
    selectionOutlineCommands.clear();
}
