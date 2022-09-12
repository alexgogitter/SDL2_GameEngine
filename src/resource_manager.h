#pragma once
#include <vector>
#include <SDL.h>
#include <string>


class resource_manager
{
    private:
        
        std::vector<SDL_Surface*> res;

    public:

        int loadSurface(std::string f_path, SDL_Surface* tex);

        void delSurface();

};