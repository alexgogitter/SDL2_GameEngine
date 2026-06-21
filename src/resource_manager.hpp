#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include <map>

typedef struct {
    std::string file_path;
    int fontSize;
} font_info;

struct texture_info
{
    std::string file_path;
    SDL_Texture* texture = nullptr;
};

struct font_resource_info
{
    std::string file_path;
    int fontSize = 0;
    TTF_Font* font = nullptr;
};

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
        
        //Texture Operations

        unsigned int loadTexture(const char * f_path);

        void loadTextures(const std::vector<std::string>& f_paths);
        
        SDL_Texture* getTexture(unsigned int texture_ID);

        void deleteTexture(unsigned int texture_ID);

        //Font Operations

        int loadFont(const char * f_path, int font_size);

        void loadFonts(const std::vector<font_info>& font_requests);

        void loadLevelResources(
            const std::vector<std::string>& texture_paths,
            const std::vector<font_info>& font_requests);

        TTF_Font* getFont(unsigned int font_ID);

        void deleteFont(unsigned int font_ID);

};

    #endif // RESOURCE_MANAGER_H