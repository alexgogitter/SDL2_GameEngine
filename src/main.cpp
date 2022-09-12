#include <iostream>
#include <stdio.h>

#include "render.h"

//Simple game design follows a Input -> Update -> draw Loop when running

int main( int argc, char* argv[] )
{
    
    Renderer r = Renderer(600, 900, 60, IMG_INIT_PNG, "Test Render Window");
    r.Renderer_Init();
    SDL_RenderClear(r.get_SDLRenderer());
    

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

#include <iostream>
#include <vector>
#include "mapReader.h"


        SDL_RenderPresent(r.get_SDLRenderer());
int mapReader::width, mapReader::height, mapReader::n;


        
    }

    /*need to work with a basic input -> update -> render System*/
	std::vector<std::vector<colorVals>> map;
	int mapWidth, mapHeight;

	std::string mapName = "res/maps/Map2.png";

	mapReader::Reader(map, mapWidth, mapHeight, mapName);
	mapReader::printMap(map);













    return 0;

}
<<<<<<< HEAD
	std::vector<std::vector<colorVals>> map;
	int mapWidth, mapHeight;

	std::string mapName = "res/maps/Map2.png";

	mapReader::Reader(map, mapWidth, mapHeight, mapName);
	mapReader::printMap(map);











    return 0;

}