# Resource Manager

`Resource_manager` is the asset cache for the engine. It loads textures and fonts once, assigns integer handles, and keeps enough metadata to reload them later if the underlying SDL object has been deleted.

## Responsibilities

- load textures from file paths
- cache texture handles by path
- load fonts by file path and size
- cache font handles by path and size pair
- preload textures and fonts for a level
- return SDL pointers on demand for rendering code

## Types

### `font_info`

A small request type used by `loadFonts()` and `loadLevelResources()`.

- `file_path`: font asset path
- `fontSize`: requested font size

### `texture_info`

Stores the source path and the current `SDL_Texture*`.

### `font_resource_info`

Stores the source path, font size, and the current `TTF_Font*`.

## Class Overview

```cpp
class Resource_manager
{
private:
    SDL_Renderer* gRenderer;

    unsigned int nextTextureId;
    unsigned int nextFontId;

    std::map<unsigned int, texture_info> textures;
    std::map<std::string, unsigned int> texture_ids_by_path;

    std::map<unsigned int, font_resource_info> fonts;
    std::map<std::string, unsigned int> font_ids_by_key;

    static std::string makeFontKey(const std::string& file_path, int font_size);

public:
    Resource_manager();
    Resource_manager(SDL_Renderer* gRenderer);

    unsigned int loadTexture(const char* f_path);
    void loadTextures(const std::vector<std::string>& f_paths);
    SDL_Texture* getTexture(unsigned int texture_ID);
    void deleteTexture(unsigned int texture_ID);

    int loadFont(const char* f_path, int font_size);
    void loadFonts(const std::vector<font_info>& font_requests);
    void loadLevelResources(
        const std::vector<std::string>& texture_paths,
        const std::vector<font_info>& font_requests);
    TTF_Font* getFont(unsigned int font_ID);
    void deleteFont(unsigned int font_ID);
};
```

## Behavior

### Texture Loading

`loadTexture()` checks whether a path has already been cached. If the texture is already live, it returns the existing handle. If the texture was cached but later released, it reloads from the stored path.

`getTexture()` follows the same cache-and-reload behavior when given a texture handle.

`deleteTexture()` destroys the SDL texture but retains the cache entry, so the handle can still be reused.

### Font Loading

`loadFont()` caches fonts by a compound key of file path and size.

`getFont()` returns the cached `TTF_Font*` or reloads it from the stored path and size if needed.

`deleteFont()` closes the font while preserving the cache entry.

### Preloading Helpers

- `loadTextures()` loads a list of texture paths.
- `loadFonts()` loads a list of font requests.
- `loadLevelResources()` is a convenience wrapper for loading both resource sets together.

## Usage Pattern

Create the resource manager after SDL has a renderer:

```cpp
Resource_manager manager(renderer);
unsigned int textureId = manager.loadTexture("res/textures/example.png");
SDL_Texture* texture = manager.getTexture(textureId);
```

For fonts:

```cpp
int fontId = manager.loadFont("res/fonts/comicz.ttf", 8);
TTF_Font* font = manager.getFont(fontId);
```

## Notes

- The resource manager assumes the renderer is valid when it loads textures.
- It currently stores raw SDL pointers and relies on explicit cleanup calls.
- `loadTexture()` and `loadFont()` return `-1` on failure, even though the handles are unsigned or stored as `int` in different places. That works in practice, but a dedicated sentinel type would be clearer.