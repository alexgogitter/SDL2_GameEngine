#pragma once
#include "engineApi.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>
#include <glm/glm.hpp>

#include "lighting2D.hpp"
#include "material2D.hpp"
#include "transform.hpp"

#include "layer.hpp"

class Camera;
class Shader;
class Window;

/// Owns 2D GPU resources and per-frame 2D draw queues.
///
/// Window owns the SDL window and OpenGL context. Components submit commands
/// to this class; they should not issue raw OpenGL.
class Renderer2D
{
public:
    ENGINE_API explicit Renderer2D(Window& window);

    /// Releases renderer-owned GPU objects. Window owns the GL context.
    ENGINE_API ~Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    /// Initializes the 2D shaders and shared quad mesh.
    ///
    /// @return Zero on success; a non-zero stage-specific error code on failure.
    ENGINE_API int Renderer_Init();

    /// Idempotently destroys 2D GPU resources.
    ENGINE_API void Renderer_Close();

    ENGINE_API void SetViewportSize(float width, float height);
    ENGINE_API void BeginFrame();

    /// Stable-sorts and executes all submitted 2D commands using submitted lights.
    /// Call after all world objects have submitted and before drawing ImGui.
    ENGINE_API void Render2D();

    /// Swaps the SDL OpenGL window backbuffer.
    /// Selects the non-owned camera used by subsequent render passes.
    ///
    /// @param camera Camera that must remain alive while active; may be nullptr.
    void SetActiveCamera(Camera* camera) { activeCamera = camera; }

    /// @return The currently active non-owned camera, or nullptr.
    Camera* GetActiveCamera() const { return activeCamera; }

    /// Submits an untextured, lit quad using a Transform2D half-size.
    ///
    /// @param transform World position, half-size scale, and rotation in radians.
    /// @param colour Linear RGBA colour with components in the 0..1 range.
    /// @param renderLayer Primary stable-sort layer; larger layers draw later.
    ENGINE_API void SubmitSolidQuad2D(
        const Transform2D& transform,
        const glm::vec4& colour,
        int renderLayer = 0,
        int objectLayer = 0
    );  

    /// Submits a textured 2D material command.
    ///
    /// @param transform World position, half-size scale, and rotation in radians.
    /// @param material Resolved GPU texture objects and material properties.
    ENGINE_API void SubmitSprite2D(
        const Transform2D& transform,
        const Material2DRenderState& material,
        int objectLayer = 0
    );


    /// Submits an unlit line-loop around a transformed quad.
    ///
    /// @param transform World position, half-size scale, and rotation in radians.
    /// @param colour Linear RGBA outline colour in the 0..1 range.
    /// @param renderLayer Primary stable-sort layer; larger layers draw later.
    ENGINE_API void SubmitOutline2D(
        const Transform2D& transform,
        const glm::vec4& colour,
        int renderLayer = 0,
        int objectLayer = 0
    );

    /// Adds a point light to the current frame, up to the sixteen-light limit.
    /// Extra lights are ignored until a later frame.
    ///
    /// @param light World-space point-light data. Shadow flags are currently reserved.
    ENGINE_API void SubmitLight2D(
        const PointLight2D& light,
        int objectLayer = 0
    );

    /// Builds a transient OpenGL text texture and submits it as an unlit quad.
    ///
    /// The command must be submitted before Render2D(). The transient texture is
    /// deleted after that render pass.
    ///
    /// @param text UTF-8 text to render.
    /// @param font Non-owned, open SDL_ttf font.
    /// @param source Reserved source rectangle; currently ignored.
    /// @param destination Optional screen/world rectangle; surface size is used when null.
    /// @param textColour SDL byte-range text colour.
    void Renderer_ttf(
        const std::string& text,
        TTF_Font* font,
        SDL_Rect* source,
        SDL_Rect* destination,
        SDL_Color& textColour
    );

    Window& GetWindow() const { return *window; }

private:
    enum class CommandType2D
    {
        Quad,
        Outline
    };

    struct RenderCommand2D
    {
        CommandType2D type = CommandType2D::Quad;
        Transform2D transform;
        Material2DRenderState material;
        std::size_t submissionOrder = 0;
        bool deleteAlbedoAfterDraw = false;
    };

    bool initialise2DRenderer();
    void destroy2DRenderer();
    void drawCommand2D(const RenderCommand2D& command);
    void uploadLights2D();
    bool shouldRenderLayer(int objectLayer) const;

    Window* window = nullptr;

    unsigned int quadVertexArray = 0;
    unsigned int quadVertexBuffer = 0;
    unsigned int quadIndexBuffer = 0;
    std::unique_ptr<Shader> spriteShader;

    Camera* activeCamera = nullptr;
    std::vector<RenderCommand2D> renderCommands2D;
    std::vector<PointLight2D> lights2D;
    std::size_t nextSubmissionOrder = 0;
};
