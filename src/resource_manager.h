#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>

typedef struct {
    const char * file_path;
    int fontSize;
} font_info;

class Resource_manager
{
    private:

        //index of file paths will match the index of the resource's pointer
        //if the requested path has no resource at the specified ID, it will reload that file.
        
        unsigned int numberOfTextures, numberOfFonts;

        SDL_Renderer* gRenderer;

        std::vector<SDL_Texture*> textures;
        std::vector<const char*> texture_paths;

        std::vector<TTF_Font*> fonts;
        std::vector<font_info*> font_paths;

    public:

        Resource_manager();

        Resource_manager(SDL_Renderer* gRenderer);
        
        //Texture Operations

        unsigned int loadTexture(const char * f_path);
        
        SDL_Texture* getTexture(unsigned int texture_ID);

        void deleteTexture(unsigned int texture_ID);

        //Font Operations

        int loadFont(const char * f_path, int font_size);

        TTF_Font* getFont(unsigned int font_ID);

        void deleteFont(unsigned int font_ID);

};