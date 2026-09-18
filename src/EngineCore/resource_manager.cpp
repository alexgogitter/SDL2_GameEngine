#include <SDL.h>
#include "resource_manager.hpp"

#include <cstdio>

#include <SDL.h>
#include <SDL_image.h>
#include <glad/glad.h>

namespace
{
unsigned int loadTextureFromPath(const std::string &filePath, TextureColourSpace colourSpace, int &width, int &height)
{
    SDL_Surface *loadedSurface = IMG_Load(filePath.c_str());
    if (loadedSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Unable to load %s: %s\n", filePath.c_str(), IMG_GetError());
        return 0;
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loadedSurface);
    if (surface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Unable to convert %s to RGBA32.\n", filePath.c_str());
        return 0;
    }

    width = surface->w;
    height = surface->h;

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, colourSpace == TextureColourSpace::SRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surface);
    return texture;
}

TTF_Font *loadFontFromPath(const std::string &filePath, int fontSize)
{
    TTF_Font *font = TTF_OpenFont(filePath.c_str(), fontSize);
    if (font == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Unable to load %s: %s\n", filePath.c_str(), TTF_GetError());
    }
    return font;
}
} // namespace

Resource_manager::Resource_manager() = default;

Resource_manager::~Resource_manager()
{
    for (auto &pair : meshes) {
        deleteMesh(pair.first);
    }

    for (auto &pair : textures) {
        if (pair.second.gpuTexture != 0) {
            glDeleteTextures(1, &pair.second.gpuTexture);
        }
    }

    for (auto &pair : fonts) {
        if (pair.second.font != nullptr) {
            TTF_CloseFont(pair.second.font);
        }
    }
}

std::string Resource_manager::makeTextureKey(const std::string &filePath, TextureColourSpace colourSpace) { return filePath + (colourSpace == TextureColourSpace::SRGB ? "#srgb" : "#linear"); }

std::string Resource_manager::makeFontKey(const std::string &filePath, int fontSize) { return filePath + "#" + std::to_string(fontSize); }

TextureHandle Resource_manager::loadTexture(const char *filePathPointer, TextureColourSpace colourSpace)
{
    if (filePathPointer == nullptr) {
        return InvalidTextureHandle;
    }

    const std::string filePath = filePathPointer;
    const std::string key = makeTextureKey(filePath, colourSpace);
    const auto cached = textureIdsByKey.find(key);
    if (cached != textureIdsByKey.end()) {
        texture_info &info = textures[cached->second];
        if (info.gpuTexture == 0) {
            info.gpuTexture = loadTextureFromPath(info.file_path, info.colourSpace, info.width, info.height);
        }
        return info.gpuTexture != 0 ? cached->second : InvalidTextureHandle;
    }

    texture_info info;
    info.file_path = filePath;
    info.colourSpace = colourSpace;
    info.gpuTexture = loadTextureFromPath(filePath, colourSpace, info.width, info.height);
    if (info.gpuTexture == 0) {
        return InvalidTextureHandle;
    }

    const TextureHandle textureId = nextTextureId++;
    textures[textureId] = info;
    textureIdsByKey[key] = textureId;
    return textureId;
}

void Resource_manager::loadTextures(const std::vector<std::string> &filePaths)
{
    for (const std::string &filePath : filePaths) {
        loadTexture(filePath.c_str());
    }
}

unsigned int Resource_manager::getTexture(TextureHandle textureId)
{
    const auto found = textures.find(textureId);
    if (found == textures.end()) {
        return 0;
    }

    texture_info &info = found->second;
    if (info.gpuTexture == 0) {
        info.gpuTexture = loadTextureFromPath(info.file_path, info.colourSpace, info.width, info.height);
    }
    return info.gpuTexture;
}

const texture_info *Resource_manager::getTextureInfo(TextureHandle textureId) const
{
    const auto found = textures.find(textureId);
    return found != textures.end() ? &found->second : nullptr;
}

void Resource_manager::deleteTexture(TextureHandle textureId)
{
    const auto found = textures.find(textureId);
    if (found != textures.end() && found->second.gpuTexture != 0) {
        glDeleteTextures(1, &found->second.gpuTexture);
        found->second.gpuTexture = 0;
    }
}

MeshHandle Resource_manager::loadBuiltInCubeMesh()
{
    constexpr const char *CubePath = "builtin://cube";

    const auto cached = meshIdsByPath.find(CubePath);

    if (cached != meshIdsByPath.end()) {
        mesh_resource_info &mesh = meshes[cached->second];

        if (mesh.vertexArray == 0 && !uploadMesh(mesh)) {
            return InvalidMeshHandle;
        }

        return cached->second;
    }

    mesh_resource_info mesh;
    mesh.assetPath = CubePath;

    mesh.vertices = {// Front: +Z
                     {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},
                     {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0}},
                     {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
                     {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1}},

                     // Back: -Z
                     {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},
                     {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}},
                     {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},
                     {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1}},

                     // Right: +X
                     {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
                     {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}},
                     {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
                     {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1}},

                     // Left: -X
                     {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
                     {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},
                     {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1}},
                     {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

                     // Top: +Y
                     {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 0}},
                     {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
                     {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
                     {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},

                     // Bottom: -Y
                     {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}},
                     {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}},
                     {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 1}},
                     {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1}}};

    for (std::uint32_t face = 0; face < 6; ++face) {
        const std::uint32_t start = face * 4;

        mesh.indices.push_back(start + 0);
        mesh.indices.push_back(start + 1);
        mesh.indices.push_back(start + 2);

        mesh.indices.push_back(start + 2);
        mesh.indices.push_back(start + 3);
        mesh.indices.push_back(start + 0);
    }

    if (!uploadMesh(mesh)) {
        return InvalidMeshHandle;
    }

    const MeshHandle handle = nextMeshId++;

    meshes.emplace(handle, std::move(mesh));
    meshIdsByPath.emplace(CubePath, handle);

    return handle;
}

bool Resource_manager::isMeshValid(MeshHandle mesh) const
{
    const auto found = meshes.find(mesh);

    return found != meshes.end() && found->second.vertexArray != 0 && !found->second.indices.empty();
}

const char *Resource_manager::getMeshAssetPath(MeshHandle mesh) const
{
    const auto found = meshes.find(mesh);

    return found != meshes.end() ? found->second.assetPath.c_str() : "";
}

std::size_t Resource_manager::getMeshVertexCount(MeshHandle mesh) const
{
    const auto found = meshes.find(mesh);

    return found != meshes.end() ? found->second.vertices.size() : 0;
}

std::size_t Resource_manager::getMeshIndexCount(MeshHandle mesh) const
{
    const auto found = meshes.find(mesh);

    return found != meshes.end() ? found->second.indices.size() : 0;
}

bool Resource_manager::getMeshRenderState(MeshHandle mesh, MeshRenderState &renderState) const
{
    renderState = {};

    const auto found = meshes.find(mesh);

    if (found == meshes.end() || found->second.vertexArray == 0 || found->second.indices.empty()) {
        return false;
    }

    renderState.vertexArray = found->second.vertexArray;

    renderState.indexCount = found->second.indices.size();

    return true;
}

void Resource_manager::deleteMesh(MeshHandle mesh)
{
    const auto found = meshes.find(mesh);

    if (found == meshes.end()) {
        return;
    }

    mesh_resource_info &resource = found->second;

    if (resource.indexBuffer != 0) {
        glDeleteBuffers(1, &resource.indexBuffer);
        resource.indexBuffer = 0;
    }

    if (resource.vertexBuffer != 0) {
        glDeleteBuffers(1, &resource.vertexBuffer);
        resource.vertexBuffer = 0;
    }

    if (resource.vertexArray != 0) {
        glDeleteVertexArrays(1, &resource.vertexArray);
        resource.vertexArray = 0;
    }
}

bool Resource_manager::uploadMesh(mesh_resource_info &mesh)
{
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        return false;
    }

    glGenVertexArrays(1, &mesh.vertexArray);
    glGenBuffers(1, &mesh.vertexBuffer);
    glGenBuffers(1, &mesh.indexBuffer);

    if (mesh.vertexArray == 0 || mesh.vertexBuffer == 0 || mesh.indexBuffer == 0) {
        return false;
    }

    glBindVertexArray(mesh.vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(MeshVertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(std::uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<const void *>(offsetof(MeshVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<const void *>(offsetof(MeshVertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<const void *>(offsetof(MeshVertex, textureCoordinate)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

int Resource_manager::loadFont(const char *filePathPointer, int fontSize)
{
    if (filePathPointer == nullptr) {
        return -1;
    }

    const std::string filePath = filePathPointer;
    const std::string key = makeFontKey(filePath, fontSize);
    const auto cached = fontIdsByKey.find(key);
    if (cached != fontIdsByKey.end()) {
        font_resource_info &info = fonts[cached->second];
        if (info.font == nullptr) {
            info.font = loadFontFromPath(info.file_path, info.fontSize);
        }
        return info.font != nullptr ? static_cast<int>(cached->second) : -1;
    }

    TTF_Font *font = loadFontFromPath(filePath, fontSize);
    if (font == nullptr) {
        return -1;
    }

    const unsigned int fontId = nextFontId++;
    fonts[fontId] = {filePath, fontSize, font};
    fontIdsByKey[key] = fontId;
    return static_cast<int>(fontId);
}

void Resource_manager::loadFonts(const std::vector<font_info> &fontRequests)
{
    for (const font_info &request : fontRequests) {
        loadFont(request.file_path.c_str(), request.fontSize);
    }
}

void Resource_manager::loadLevelResources(const std::vector<std::string> &texturePaths, const std::vector<font_info> &fontRequests)
{
    loadTextures(texturePaths);
    loadFonts(fontRequests);
}

TTF_Font *Resource_manager::getFont(unsigned int fontId)
{
    const auto found = fonts.find(fontId);
    if (found == fonts.end()) {
        return nullptr;
    }

    font_resource_info &info = found->second;
    if (info.font == nullptr) {
        info.font = loadFontFromPath(info.file_path, info.fontSize);
    }
    return info.font;
}

void Resource_manager::deleteFont(unsigned int fontId)
{
    const auto found = fonts.find(fontId);
    if (found != fonts.end() && found->second.font != nullptr) {
        TTF_CloseFont(found->second.font);
        found->second.font = nullptr;
    }
}
