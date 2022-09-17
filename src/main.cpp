#include <iostream>
#include <stdio.h>
#include <vector>

#include "mapReader.h"
#include "render.h"
#include "resource_manager.h"

//Simple game design follows a Input -> Update -> draw Loop when running


int mapReader::width, mapReader::height, mapReader::n;


int main( int argc, char* argv[] )
{
    
    Renderer r = Renderer(600, 900, 60, IMG_INIT_PNG, "Test Render Window");
    r.Renderer_Init();
    SDL_RenderClear(r.get_SDLRenderer());

    Resource_manager manager(r.get_SDLRenderer());
    int test_tile=manager.loadTexture("res/textures/test/test_tileset1.png");
    Object o = Object(test_tile, 32, 32, manager);
    manager.deleteTexture(test_tile);

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    //While application is running
    while( !quit )
    {   
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                r.Renderer_Close();
                quit = true;
            }
        }
        
        SDL_RenderClear(r.get_SDLRenderer());

        o.draw(r.get_SDLRenderer());

        SDL_RenderPresent(r.get_SDLRenderer());

    }

    /*need to work with a basic input -> update -> render System*/
	// std::vector<std::vector<colorVals>> map;
	// int mapWidth, mapHeight;

	// std::string mapName = "res/maps/Map2.png";

	// mapReader::Reader(map, mapWidth, mapHeight, mapName);
	// mapReader::printMap(map);


    return 0;

}