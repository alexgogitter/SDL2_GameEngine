#pragma once

#include <SDL.h>
#include <vector>

typedef struct {
    SDL_Texture* tileset_piece;
    float x, y;
} tile;


class scene{

    private:

        std::vector<std::vector<tile>> tile_map;
        unsigned int tileSurface;

    public:

        scene();

        ~scene();

};

