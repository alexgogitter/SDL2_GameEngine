#pragma once
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include "resource_manager.h"

class Object
{
    
private:

    bool collisions, sprite_sht; 

    int cur_anim_tick=0, cur_anim=0;
    int spriteID;

    SDL_Renderer* renderer=NULL;
    SDL_Texture* tex=NULL;

    SDL_Rect collider_box;
    SDL_Rect sprite_clipping_rect;

public:

    Object(unsigned int texture_ID, int texture_width, int texture_height, Resource_manager r);

    void update();

    void draw(SDL_Renderer* r);

    const SDL_Rect* get_Collider(){return &collider_box;}
        
    SDL_Texture* get_Texture(){return tex;}

};