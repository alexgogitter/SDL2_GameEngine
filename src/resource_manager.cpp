#include "resource_manager.hpp"

#include <stdio.h>

namespace
{
    SDL_Texture* loadTextureFromPath(SDL_Renderer* renderer, const std::string& file_path)
    {
        SDL_Surface* surface = IMG_Load(file_path.c_str());
        if (!surface)
        {
            fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s\n", file_path.c_str(), IMG_GetError());
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture)
        {
            fprintf(stderr, "ERROR: Unable to create texture from %s. SDL_ERROR: %s\n", file_path.c_str(), SDL_GetError());
        }

        SDL_FreeSurface(surface);
        return texture;
    }

    TTF_Font* loadFontFromPath(const std::string& file_path, int font_size)
    {
        TTF_Font* font = TTF_OpenFont(file_path.c_str(), font_size);
        if (!font)
        {
            fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s\n", file_path.c_str(), TTF_GetError());
        }

        return font;
    }
}

Resource_manager::Resource_manager()
    : gRenderer(nullptr), nextTextureId(0), nextFontId(0)
{
}

Resource_manager::Resource_manager(SDL_Renderer* ren)
    : gRenderer(ren), nextTextureId(0), nextFontId(0)
{
}

std::string Resource_manager::makeFontKey(const std::string& file_path, int font_size)
{
    return file_path + "#" + std::to_string(font_size);
}

unsigned int Resource_manager::loadTexture(const char * f_path)
{
    if (!f_path)
    {
        fprintf(stderr, "ERROR: Unable to load texture from a null path.\n");
        return -1;
    }

    std::string file_path = f_path;
    auto cached_texture = texture_ids_by_path.find(file_path);
    if (cached_texture != texture_ids_by_path.end())
    {
        auto texture_it = textures.find(cached_texture->second);
        if (texture_it != textures.end() && texture_it->second.texture)
        {
            return cached_texture->second;
        }

        SDL_Texture* texture = loadTextureFromPath(gRenderer, file_path);
        if (!texture)
        {
            return -1;
        }

        texture_it->second.texture = texture;
        return cached_texture->second;
    }

    SDL_Texture* texture = loadTextureFromPath(gRenderer, file_path);
    if (!texture)
    {
        return -1;
    }

    unsigned int texture_id = nextTextureId++;
    textures[texture_id] = texture_info{file_path, texture};
    texture_ids_by_path[file_path] = texture_id;
    return texture_id;
}

void Resource_manager::loadTextures(const std::vector<std::string>& f_paths)
{
    for (const auto& file_path : f_paths)
    {
        loadTexture(file_path.c_str());
    }
}


SDL_Texture* Resource_manager::getTexture(unsigned int texture_ID)
{
    auto texture_it = textures.find(texture_ID);
    if (texture_it == textures.end())
    {
        fprintf(stderr, "ERROR: Invalid texture handle: %u.\n", texture_ID);
        return nullptr;
    }

    if (texture_it->second.texture)
    {
        return texture_it->second.texture;
    }

    fprintf(stdout, "COULD NOT MOUNT TEXTURE FROM HANDLE. RELOADING\n");

    SDL_Texture* texture = loadTextureFromPath(gRenderer, texture_it->second.file_path);
    if (!texture)
    {
        return nullptr;
    }

    texture_it->second.texture = texture;
    return texture;
}

void Resource_manager::deleteTexture(unsigned int texture_ID)
{
    auto texture_it = textures.find(texture_ID);
    if (texture_it == textures.end())
    {
        return;
    }

    if (texture_it->second.texture)
    {
        SDL_DestroyTexture(texture_it->second.texture);
        texture_it->second.texture = nullptr;
    }
}

//Font Operations

int Resource_manager::loadFont(const char* f_path, int font_size)
{
    if (!f_path)
    {
        fprintf(stderr, "ERROR: Unable to load font from a null path.\n");
        return -1;
    }

    std::string file_path = f_path;
    std::string font_key = makeFontKey(file_path, font_size);

    auto cached_font = font_ids_by_key.find(font_key);
    if (cached_font != font_ids_by_key.end())
    {
        auto font_it = fonts.find(cached_font->second);
        if (font_it != fonts.end() && font_it->second.font)
        {
            return static_cast<int>(cached_font->second);
        }

        TTF_Font* font = loadFontFromPath(file_path, font_size);
        if (!font)
        {
            return -1;
        }

        font_it->second.font = font;
        return static_cast<int>(cached_font->second);
    }

    TTF_Font* font = loadFontFromPath(file_path, font_size);
    if (!font)
    {
        return -1;
    }

    unsigned int font_id = nextFontId++;
    fonts[font_id] = font_resource_info{file_path, font_size, font};
    font_ids_by_key[font_key] = font_id;
    return static_cast<int>(font_id);
}

void Resource_manager::loadFonts(const std::vector<font_info>& font_requests)
{
    for (const auto& request : font_requests)
    {
        loadFont(request.file_path.c_str(), request.fontSize);
    }
}

void Resource_manager::loadLevelResources(
    const std::vector<std::string>& texture_paths,
    const std::vector<font_info>& font_requests)
{
    loadTextures(texture_paths);
    loadFonts(font_requests);
}

TTF_Font* Resource_manager::getFont(unsigned int font_ID)
{
    auto font_it = fonts.find(font_ID);
    if (font_it == fonts.end())
    {
        fprintf(stderr, "ERROR: Invalid font handle: %u.\n", font_ID);
        return nullptr;
    }

    if (font_it->second.font)
    {
        return font_it->second.font;
    }

    TTF_Font* font = loadFontFromPath(font_it->second.file_path, font_it->second.fontSize);
    if (!font)
    {
        return nullptr;
    }

    font_it->second.font = font;
    return font;
}

void Resource_manager::deleteFont(unsigned int font_ID)
{
    auto font_it = fonts.find(font_ID);
    if (font_it == fonts.end())
    {
        return;
    }

    if (font_it->second.font)
    {
        TTF_CloseFont(font_it->second.font);
        font_it->second.font = nullptr;
    }
}