#pragma once

#include "engineApi.hpp"
#include "mesh3D.hpp"
#include "layer.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Camera;
class Shader;
class Window;

/// Owns the 3D GPU programs and per-frame mesh submission queues.
class Renderer3D
{
public:
	ENGINE_API explicit Renderer3D(Window& window);
	ENGINE_API ~Renderer3D();

	Renderer3D(const Renderer3D&) = delete;
	Renderer3D& operator=(const Renderer3D&) = delete;

	ENGINE_API bool Initialize();
	ENGINE_API void Shutdown();
	ENGINE_API void BeginFrame();
	ENGINE_API void SetViewportSize(float width, float height);

	void SetActiveCamera(Camera* camera) { activeCamera = camera; }
	Camera* GetActiveCamera() const { return activeCamera; }

	ENGINE_API void SubmitMesh(
		const MeshRenderState& mesh,
		const glm::mat4& worldMatrix,
		const glm::vec4& colour,
		int objectLayer = 0
	);

	ENGINE_API void SubmitSelectionOutline(
		const MeshRenderState& mesh,
		const glm::mat4& worldMatrix,
		const glm::vec4& colour,
		float shellScale = 1.04f,
		int objectLayer = 0
	);

	ENGINE_API void Render3D();

private:
	struct RenderCommand
	{
		unsigned int vertexArray = 0;
		std::size_t indexCount = 0;
		glm::mat4 worldMatrix = glm::mat4(1.0f);
		glm::vec4 colour = glm::vec4(1.0f);
	};

	struct SelectionOutlineCommand
	{
		unsigned int vertexArray = 0;
		std::size_t indexCount = 0;
		glm::mat4 worldMatrix = glm::mat4(1.0f);
		glm::vec4 colour = glm::vec4(1.0f);
		float shellScale = 1.04f;
	};

	bool shouldRenderLayer(int objectLayer) const;

	Window* window = nullptr;
	Camera* activeCamera = nullptr;
	std::unique_ptr<Shader> meshShader;
	std::unique_ptr<Shader> meshOutlineShader;
	std::vector<RenderCommand> renderCommands;
	std::vector<SelectionOutlineCommand> selectionOutlineCommands;
};
