#pragma once
#include "engineApi.hpp"
#include <map>
#include <string>
#include <vector>

#include <SDL_ttf.h>
#include <cstdint>
#include <cstddef>
#include "mesh3D.hpp"
#include "material2D.hpp"

struct font_info
{
    std::string file_path;
    int fontSize = 0;
};

enum class TextureColourSpace
{
    Linear,
    SRGB
};

struct mesh_resource_info
{
    std::string assetPath;

    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;

    unsigned int vertexArray = 0;
    unsigned int vertexBuffer = 0;
    unsigned int indexBuffer = 0;
};

/// Metadata and GPU state associated with one cached texture handle.
struct texture_info
{
    std::string file_path;
    unsigned int gpuTexture = 0;
    int width = 0;
    int height = 0;
    TextureColourSpace colourSpace = TextureColourSpace::SRGB;
};

struct font_resource_info
{
    std::string file_path;
    int fontSize = 0;
    TTF_Font* font = nullptr;
};

class Resource_manager
{
public:
    /// Creates an empty cache. A current OpenGL context is required before loading textures.
    ENGINE_API Resource_manager();

    /// Deletes cached OpenGL textures and closes cached fonts.
    /// The renderer's OpenGL context must still be current.
    ENGINE_API ~Resource_manager();

    Resource_manager(const Resource_manager&) = delete;
    Resource_manager& operator=(const Resource_manager&) = delete;

    /// Decodes an image with SDL_image and uploads it to the active OpenGL context.
    ///
    /// Cache identity includes both path and colour space.
    ///
    /// @param filePath Path relative to the process working directory.
    /// @param colourSpace SRGB for colour textures; Linear for numerical data maps.
    /// @return Stable texture handle, or InvalidTextureHandle on failure.
    ENGINE_API TextureHandle loadTexture(
        const char* filePath,
        TextureColourSpace colourSpace = TextureColourSpace::SRGB
    );

    /// Preloads sRGB textures from a list of paths.
    /// @param filePaths Paths relative to the process working directory.
    ENGINE_API void loadTextures(const std::vector<std::string>& filePaths);

    /// Resolves a stable handle to an OpenGL texture object, reloading if released.
    /// @param textureId Handle returned by loadTexture().
    /// @return OpenGL texture object name, or zero for an invalid/failed handle.
    ENGINE_API unsigned int getTexture(TextureHandle textureId);

    /// Looks up immutable metadata without reloading the GPU object.
    /// @param textureId Handle returned by loadTexture().
    /// @return Pointer owned by the manager, or nullptr when the handle is unknown.
    ENGINE_API const texture_info* getTextureInfo(TextureHandle textureId) const;

    /// Deletes the GPU object while retaining path and handle metadata for reload.
    /// @param textureId Handle returned by loadTexture().
    ENGINE_API void deleteTexture(TextureHandle textureId);

        /// Creates or retrieves the reusable built-in cube mesh.
    ENGINE_API MeshHandle loadBuiltInCubeMesh();

    ENGINE_API bool isMeshValid(MeshHandle mesh) const;

    ENGINE_API const char* getMeshAssetPath(
        MeshHandle mesh
    ) const;

    ENGINE_API std::size_t getMeshVertexCount(
        MeshHandle mesh
    ) const;

    ENGINE_API std::size_t getMeshIndexCount(
        MeshHandle mesh
    ) const;

    ENGINE_API bool getMeshRenderState(
        MeshHandle mesh,
        MeshRenderState& renderState
    ) const;

    ENGINE_API void deleteMesh(MeshHandle mesh);

    /// Opens and caches an SDL_ttf font by path and point size.
    /// @param filePath Path relative to the process working directory.
    /// @param fontSize Requested point size.
    /// @return Stable non-negative font handle, or -1 on failure.
    ENGINE_API int loadFont(const char* filePath, int fontSize);

    /// Preloads a collection of font requests.
    ENGINE_API void loadFonts(const std::vector<font_info>& fontRequests);

    /// Convenience operation that preloads sRGB textures and fonts.
    ENGINE_API void loadLevelResources(
        const std::vector<std::string>& texturePaths,
        const std::vector<font_info>& fontRequests
    );

    /// Resolves a font handle, reopening the font if it was explicitly released.
    /// @param fontId Non-negative handle returned by loadFont().
    /// @return Manager-owned TTF_Font pointer, or nullptr on failure.
    ENGINE_API TTF_Font* getFont(unsigned int fontId);

    /// Closes a cached font while preserving metadata for a later reopen.
    /// @param fontId Non-negative handle returned by loadFont().
    ENGINE_API void deleteFont(unsigned int fontId);

private:
    static std::string makeTextureKey(
        const std::string& filePath,
        TextureColourSpace colourSpace
    );
    static std::string makeFontKey(const std::string& filePath, int fontSize);

    TextureHandle nextTextureId = 0;
    unsigned int nextFontId = 0;

    std::map<TextureHandle, texture_info> textures;
    std::map<std::string, TextureHandle> textureIdsByKey;
    std::map<unsigned int, font_resource_info> fonts;
    std::map<std::string, unsigned int> fontIdsByKey;

    static bool uploadMesh(mesh_resource_info& mesh);

    MeshHandle nextMeshId = 0;

    std::map<MeshHandle, mesh_resource_info> meshes;
    std::map<std::string, MeshHandle> meshIdsByPath;

};
