#pragma once
#include "object.h"
#include "resource_manager.h"

Object::Object(const char* sprite_path, int col_width, int col_height, bool col, SDL_Renderer* ren)
{
    SDL_Surface* sprite_surf=IMG_Load(sprite_path);
    if(!sprite_surf)
    {
        printf("Unable to load \"%s\" ERRROR: %s", sprite_path, IMG_GetError());

    }else{
        tex=SDL_CreateTextureFromSurface(ren, sprite_surf);
        if(!tex)
        {
            printf("Unable to generate a texture of \"%s\" ERROR: %s", sprite_path, SDL_GetError());
        }else{
            collider_box={0, 0, col_width, col_height};
        }
    }
    
    SDL_FreeSurface(sprite_surf);

}

Object::Object(const char* sprite_path, int spr_width, int spr_height, int col_width, int col_height, bool col, SDL_Renderer* ren)
{   
    sprite_sht=true;
    SDL_Surface* sprite_surf=IMG_Load(sprite_path);
    if(!sprite_surf)
    {
        printf("Unable to load \"%s\" ERRROR: %s", sprite_path, IMG_GetError());

    }else{
        tex=SDL_CreateTextureFromSurface(ren, sprite_surf);
        if(!tex)
        {
            printf("Unable to generate a texture of \"%s\" ERROR: %s", sprite_path, SDL_GetError());
        }else{
            collider_box={0, 0, col_width, col_height};
        }
    }

}

Object::Object(int sprite_ID, int sprite_width, int sprite_height, int col_width, int col_height, Resource_manager r)
{
    sprite_sht=true;
    if(sprite_ID<0)
    {
        fprintf(stderr, "Unable to mount texture with ID %d\n", sprite_ID);
    }
    else
    {

        spriteID=sprite_ID;
        tex=r.getTexture(spriteID);

        collider_box={0, 0, col_width, col_height};

    }
    

     
}

void Object::draw(SDL_Renderer* r)
{
    if(sprite_sht)
        SDL_RenderCopy(r, tex, &sprite_clipping_rect, &collider_box);
    else
        SDL_RenderCopy(r, tex, NULL, &collider_box);
}

void Object::update()
{

}
