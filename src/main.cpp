#include <iostream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <math.h>

#include "mapReader.h"
#include "render.h"

//Simple game design follows a Input -> Update -> draw Loop when running



int mapReader::width, mapReader::height, mapReader::n;

std::vector<std::vector<colorVals>> map;
int mapWidth, mapHeight;
std::string mapName = "res/maps/Map2.png";

const int tileSize = 20;
const int mapStartX = 1, mapStartY = 1;

// Time constants
Uint64 deltaTime = 0;
Uint64 lastFrame = 0;



int main( int argc, char* argv[] )
{
    
	mapReader::Reader(map, mapWidth, mapHeight, mapName);





    Renderer r = Renderer(1000, 1000, 60, IMG_INIT_PNG, "Test Render Window");
    r.Renderer_Init();
    SDL_RenderClear(r.get_SDLRenderer());
    
    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    //While application is running
    while( !quit )
    {  

		Uint64 currentFrame = SDL_GetTicks64();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;




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


		

		// Rendering map
		for (int i = 0; i < map.size(); i++) {
			for (int j = 0; j < map[0].size(); j++) {
				
				SDL_Rect rect1;
				rect1.y = mapStartY+i*tileSize; rect1.x = mapStartX+j*tileSize; 
				rect1.w = tileSize;
				rect1.h = tileSize;

				SDL_SetRenderDrawColor(r.get_SDLRenderer(), map[i][j].r, map[i][j].g, map[i][j].b, 255);
				SDL_RenderDrawRect(r.get_SDLRenderer(), &rect1);
				SDL_RenderFillRect(r.get_SDLRenderer(), &rect1);


			}
		}

		std::ostringstream oss;
		oss << deltaTime;

		SDL_Color GREEN = {0, 255, 50, 255};
		// r.Renderer_ttf(oss.str(), static_cast<std::string>("comicz.ttf"), 24, GREEN);
		r.Renderer_ttf("HELLO!!!", static_cast<std::string>("comicz.ttf"), static_cast<int>(sin(SDL_GetTicks64())), GREEN);


		SDL_SetRenderDrawColor(r.get_SDLRenderer(), 200, 200, 200, 255);






        SDL_RenderPresent(r.get_SDLRenderer());

    }

    /*need to work with a basic input -> update -> render System*/

	// mapReader::printMap(map);

    return 0;

}