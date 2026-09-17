#include "interfaceImplementation.hpp"

#include <cstddef>
#include <cstdint>

#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>

#include "shader.hpp"

#include "editorDockspace.hpp"

Interface *Interface::instance = nullptr;

Interface::Interface(SDL_Window *applicationWindow, SDL_GLContext applicationContext) : window(applicationWindow), glContext(applicationContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.BackendRendererName = "SDL2_GameEngine_OpenGL3";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    initialiseOpenGLRenderer();
}

Interface *Interface::create(SDL_Window *window, SDL_GLContext glContext)
{
    if (instance == nullptr) {
        instance = new Interface(window, glContext);
    }
    return instance;
}

Interface *Interface::get() { return instance; }

Interface::~Interface()
{
    SDL_GL_MakeCurrent(window, glContext);

    if (fontTexture != 0) {
        glDeleteTextures(1, &fontTexture);
    }
    if (indexBuffer != 0) {
        glDeleteBuffers(1, &indexBuffer);
    }
    if (vertexBuffer != 0) {
        glDeleteBuffers(1, &vertexBuffer);
    }
    if (vertexArray != 0) {
        glDeleteVertexArrays(1, &vertexArray);
    }
    shader.reset();

    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    instance = nullptr;
}

bool Interface::initialiseOpenGLRenderer()
{
    static const char *vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPosition;
        layout (location = 1) in vec2 aTextureCoordinate;
        layout (location = 2) in vec4 aColour;

        uniform mat4 uProjection;

        out vec2 vTextureCoordinate;
        out vec4 vColour;

        void main()
        {
            vTextureCoordinate = aTextureCoordinate;
            vColour = aColour;
            gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
        }
    )";

    static const char *fragmentSource = R"(
        #version 330 core
        in vec2 vTextureCoordinate;
        in vec4 vColour;
        uniform sampler2D uTexture;
        out vec4 fragmentColour;

        void main()
        {
            fragmentColour = vColour * texture(uTexture, vTextureCoordinate);
        }
    )";

    shader = std::make_unique<Shader>();
    if (!shader->loadFromSource(vertexSource, fragmentSource)) {
        return false;
    }
    shader->use();
    shader->setInt("uTexture", 0);

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<void *>(offsetof(ImDrawVert, pos)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<void *>(offsetof(ImDrawVert, uv)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<void *>(offsetof(ImDrawVert, col)));
    glBindVertexArray(0);

    unsigned char *pixels = nullptr;
    int width = 0;
    int height = 0;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    ImGui::GetIO().Fonts->SetTexID(static_cast<ImTextureID>(static_cast<intptr_t>(fontTexture)));
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Interface::draw()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    DrawEditorDockspace();

    for (const auto &callback : drawCallbacks) {
        callback();
    }

    ImGui::Render();
    renderDrawData(ImGui::GetDrawData());
}

void Interface::renderDrawData(ImDrawData *drawData)
{
    const int framebufferWidth = static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    const int framebufferHeight = static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // ImGui vertex colours are authored for direct display rather than the
    // engine's linear-light material pipeline.
    glDisable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_SCISSOR_TEST);

    shader->use();
    const float left = drawData->DisplayPos.x;
    const float right = drawData->DisplayPos.x + drawData->DisplaySize.x;
    const float top = drawData->DisplayPos.y;
    const float bottom = drawData->DisplayPos.y + drawData->DisplaySize.y;
    shader->setMat4("uProjection", glm::ortho(left, right, bottom, top));
    glBindVertexArray(vertexArray);

    const ImVec2 clipOffset = drawData->DisplayPos;
    const ImVec2 clipScale = drawData->FramebufferScale;

    for (int listIndex = 0; listIndex < drawData->CmdListsCount; ++listIndex) {
        const ImDrawList *commandList = drawData->CmdLists[listIndex];
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, commandList->VtxBuffer.Size * sizeof(ImDrawVert), commandList->VtxBuffer.Data, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, commandList->IdxBuffer.Size * sizeof(ImDrawIdx), commandList->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int commandIndex = 0; commandIndex < commandList->CmdBuffer.Size; ++commandIndex) {
            const ImDrawCmd *command = &commandList->CmdBuffer[commandIndex];
            if (command->UserCallback != nullptr) {
                if (command->UserCallback != ImDrawCallback_ResetRenderState) {
                    command->UserCallback(commandList, command);
                }
                continue;
            }

            const ImVec2 clipMinimum = {(command->ClipRect.x - clipOffset.x) * clipScale.x, (command->ClipRect.y - clipOffset.y) * clipScale.y};
            const ImVec2 clipMaximum = {(command->ClipRect.z - clipOffset.x) * clipScale.x, (command->ClipRect.w - clipOffset.y) * clipScale.y};
            if (clipMaximum.x <= clipMinimum.x || clipMaximum.y <= clipMinimum.y) {
                continue;
            }

            glScissor(static_cast<int>(clipMinimum.x), static_cast<int>(framebufferHeight - clipMaximum.y), static_cast<int>(clipMaximum.x - clipMinimum.x), static_cast<int>(clipMaximum.y - clipMinimum.y));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, static_cast<unsigned int>(static_cast<intptr_t>(command->GetTexID())));

            glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(command->ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void *>(command->IdxOffset * sizeof(ImDrawIdx)), static_cast<GLint>(command->VtxOffset));
        }
    }

    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
}

void Interface::addDrawCallback(std::function<void()> callback) { drawCallbacks.push_back(callback); }

void Interface::update(SDL_Event &event) { ImGui_ImplSDL2_ProcessEvent(&event); }
