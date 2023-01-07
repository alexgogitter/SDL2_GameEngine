#include "resource_manager.h"

Resource_manager::Resource_manager(SDL_Renderer* ren)
    : gRenderer(ren)
{
    numberOfFonts=0;
    numberOfTextures=0;
}

unsigned int Resource_manager::loadTexture(const char * f_path)
{
    
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
            fprintf(stdout, "COULD NOT MOUNT TEXTURE FROM HANDLE. RELOADING\n");

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
    if(texture_ID>=0&&texture_ID<this->numberOfTextures)
    {
        if(textures[texture_ID])
        {
            //We hit a texture.
            //Now it's time to free it.
            SDL_DestroyTexture(textures[texture_ID]);
            textures[texture_ID]=NULL;
        }
    }
}

//Font Operations

int Resource_manager::loadFont(const char* f_path, int font_size)
{
    TTF_Font* font = NULL;
    font=TTF_OpenFont(f_path, font_size);
    if(!font)
    {
        fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s", f_path, SDL_GetError());   
        TTF_CloseFont(font);
        font=NULL;
        return -1;
    }
    else
    {
        
        font_info font_data = {f_path, font_size};
        numberOfTextures++;
        fonts.push_back(font);
        font_paths.push_back(&font_data);
        return numberOfTextures-1;

    }
}

TTF_Font* Resource_manager::getFont(unsigned int font_ID)
{
    if(font_ID>=0&&font_ID<this->numberOfFonts)
    {
        if(fonts[font_ID])
        {
            return fonts[font_ID];
        }
        else
        {
            TTF_Font* font = NULL;
            font=TTF_OpenFont(font_paths[font_ID]->file_path, font_paths[font_ID]->fontSize);
            if(!font)
            {
                fprintf(stderr, "ERROR: Unable to load %s. SDL_ERROR: %s", font_paths[font_ID]->file_path, SDL_GetError());   
                TTF_CloseFont(font);
                font=NULL;

            }
            else
            {
                numberOfTextures++;
                fonts[font_ID]=font;
                return font;

            }
                
        }
    }

    return NULL;

}

void Resource_manager::deleteFont(unsigned int font_ID)
{
    if(font_ID>=0&&font_ID<this->numberOfFonts)
    {
        if(fonts[font_ID])
        {
            //We hit a texture.
            //Now it's time to free it.
            TTF_CloseFont(fonts[font_ID]);
            fonts[font_ID]=NULL;
        }
    }
}