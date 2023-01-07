#pragma once
#include "object.h"



Object::Object(unsigned int texture_handle, int texture_width, int texture_height, Resource_manager r)
{
    tex = r.getTexture(texture_handle);
    if(!tex)
    {
        fprintf(stderr, "ERROR: Cannot mount texture from handle: %d\n", texture_handle);
    }else{
        collider_box={0, 0, texture_width, texture_height};
    }
    
     
}

void Object::draw(SDL_Renderer* r)
{
    SDL_RenderCopy(r, tex, NULL, &collider_box);
}

void Object::update()
{

}
