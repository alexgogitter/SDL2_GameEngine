#include "resource_manager.h"

Resource_manager::Resource_manager(SDL_Renderer* ren)
    : gRenderer(ren)
{
    numberOfFonts=0;
    numberOfTextures=0;
}

unsigned int Resource_manager::loadTexture(const char * f_path)
{
    // numberOfTextures=numberOfTextures+1;
    SDL_Surface* surf = NULL;
    surf=IMG_Load(f_path);
    if(!surf)
    {
        fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s", f_path, SDL_GetError());   
        SDL_FreeSurface(surf);
        return -1;
    }
    else
    {
        SDL_Texture* tex=NULL;
        tex=SDL_CreateTextureFromSurface(gRenderer, surf);
        if(!tex)
        {
            fprintf(stderr, "ERROR: Unable to create texture from %s. SDL_ERROR: %s", f_path, SDL_GetError());
            SDL_FreeSurface(surf);
            return -1;
        }
        else
        {
            numberOfTextures++;
            textures.push_back(tex);
            texture_paths.push_back(f_path);
            SDL_FreeSurface(surf);
            return numberOfTextures-1;
           
        }

    }
}


SDL_Texture* Resource_manager::getTexture(unsigned int texture_ID)
{
    // if(texture_ID>=0&&texture_ID<this->numberOfTextures)
    if(texture_ID>=0&&texture_ID<this->numberOfTextures)
    {
        //Texture ID is valid. Now needs to check if there is a valid SDL_Texture at that position
        //in the vector. 
        if(textures[texture_ID])
        {
            //Now we checked there is a valid texture we can return it.
            return textures[texture_ID];
        }
        else
        {
            SDL_Surface* surf = NULL;
            surf=IMG_Load(texture_paths[texture_ID]);
            if(!surf)
            {
                fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s", texture_paths[texture_ID], SDL_GetError());   
                SDL_FreeSurface(surf);
                return NULL;
            }
            else
            {
                SDL_Texture* tex=NULL;
                tex=SDL_CreateTextureFromSurface(gRenderer, surf);
                if(!tex)
                {
                    fprintf(stderr, "ERROR: Unable to create texture from %s. SDL_ERROR: %s", texture_paths[texture_ID], SDL_GetError());
                    SDL_FreeSurface(surf);
                    return NULL;
                }
                else
                {
                    textures[texture_ID]=tex;
                    SDL_FreeSurface(surf);
                    return tex;
                }

            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Invalid file Handle: %d.\n", texture_ID);
        return NULL;
    }
}

void Resource_manager::deleteTexture(unsigned int texture_ID)
{

}

//Font Operations

int Resource_manager::loadFont(const char* f_path)
{
    return 0;
}

TTF_Font* Resource_manager::getFont(unsigned int font_ID)
{
    return NULL;
}

void Resource_manager::deleteFont(unsigned int font_ID)
{

}