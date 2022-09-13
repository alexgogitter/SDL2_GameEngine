#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>



class Resource_manager
{
    private:

        //index of file paths will match the index of the resource's pointer
        //if the requested path has no resource at the specified ID, it will reload that file.
        
        int numberOfTextures = 0, numberOfFonts = 0;

        SDL_Renderer* gRenderer=NULL;

        std::vector<SDL_Texture*> textures;
        std::vector<const char*> texture_paths;

        std::vector<TTF_Font*> fonts;
        std::vector<const char*> font_paths;

    public:

        Resource_manager();

        Resource_manager(SDL_Renderer* gRenderer);
        
        //Texture Operations

        int loadTexture(const char * f_path);
        
        SDL_Texture* getTexture(int texture_ID);

        void deleteTexture(int texture_ID);

        //Font Operations

        int loadFont(const char * f_path);

        TTF_Font* getFont(int font_ID);

        void deleteFont(int font_ID);

};