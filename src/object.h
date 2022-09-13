#pragma once
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

class Object
{
    
private:

    bool collisions, sprite_sht;
    const char* sprite_load_path;

    int cur_anim_tick=0, cur_anim=0;
    int spriteID;

    SDL_Renderer* renderer=NULL;
    SDL_Texture* tex=NULL;

    SDL_Rect collider_box;
    SDL_Rect sprite_clipping_rect;

public:

    Object(const char* sprite_path, int col_width, int col_height, bool col, SDL_Renderer* ren);

    Object(const char* sprite_path, int spr_width, int spr_height, int col_width, int col_height, bool col, SDL_Renderer* ren);

    Object(int sprite_ID, int sprite_width, int sprite_height, int column_width, int col_height, Resource_manager r);

    void update();

    void draw(SDL_Renderer* r);

    const SDL_Rect* get_Collider(){return &collider_box;}
        
    SDL_Texture* get_Texture(){return tex;}

};