#include <iostream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <math.h>

// #include "mapReader.h"
#include "fpsCounter.h"
#include "render.h"
#include "resource_manager.h"
#include "time.h"


//Simple game design follows a Input -> Update -> draw Loop when running


// int mapReader::width, mapReader::height, mapReader::n;


// std::vector<std::vector<colorVals>> map;
int mapWidth, mapHeight;
std::string mapName = "res/maps/Map2.png";
std::string fontPath = "res/fonts/comicz.ttf";

const int tileSize = 5;
const int mapStartX = 0, mapStartY = 0;



// Time constants
Uint64 deltaTime = 0;
Uint64 lastFrame = 0;

int fpsMSTotal = 0;
int fpsCount = 0;
float fps;


int main( int argc, char* argv[] )
{

    
	// mapReader::Reader(map, mapWidth, mapHeight, mapName);





    Renderer r = Renderer(1000, 1000, 60, IMG_INIT_PNG, "Test Render Window");
    r.Renderer_Init();


	// Alex testing rendering png from test textures
    SDL_RenderClear(r.get_SDLRenderer());

    Time timer = Time();

    Resource_manager manager(r.get_SDLRenderer());
    int test_tile=manager.loadTexture("res/textures/test/test_tileset1.png");
    Object o = Object(test_tile, 32, 32, manager);
    manager.deleteTexture(test_tile);

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;


	// Map rendering
	SDL_Surface* surface = IMG_Load(mapName.c_str());
	if (surface == nullptr){
		fprintf(stderr, "Surface Load Error");
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(r.get_SDLRenderer(), surface);
	if (texture == nullptr){
		fprintf(stderr, "Texture Load Error");
	}
	SDL_FreeSurface(surface);
	
	fpsCounter fpsCountObj = fpsCounter();

	SDL_Rect* mapRect = new SDL_Rect{100,100,400,100};

    //While application is running
    while( !quit )
    {  
		SDL_RenderClear(r.get_SDLRenderer());

		Uint64 currentFrame = SDL_GetTicks64();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		if (SDL_RenderCopy(r.get_SDLRenderer(), texture, NULL, mapRect) != 0){
			fprintf(stderr, "RenderCopyError: %s", SDL_GetError());
		}


    uint64_t current_tick;

    //While application is running
    while( !quit )
    {
        timer.tick();
        

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


		// Rendering map // SLOW AS 
		// for (int i = 0; i < map.size(); i++) {
		// 	for (int j = 0; j < map[0].size(); j++) {
				
		// 		SDL_Rect rect1;
		// 		rect1.y = mapStartY+i*tileSize; rect1.x = mapStartX+j*tileSize; 
		// 		rect1.w = tileSize;
		// 		rect1.h = tileSize;

		// 		SDL_SetRenderDrawColor(r.get_SDLRenderer(), map[i][j].r, map[i][j].g, map[i][j].b, 255);
		// 		SDL_RenderDrawRect(r.get_SDLRenderer(), &rect1);
		// 		SDL_RenderFillRect(r.get_SDLRenderer(), &rect1);
		// 	}
		// }


		// std::cout << fpsCount << std::endl;
		if (fpsCount == 100){
			std::cout << "FPSMSTOTAL: [" << fpsMSTotal << "] FPS: " << fps << std::endl;
			
			fps = (1000.0f / ((float)(fpsMSTotal) / 100));
			// std::cout << (fpsMSTotal) / 1000.0f << std::endl;
			fpsCount = 0; fpsMSTotal = 0;
		}
		fpsCount += 1;
		fpsMSTotal += deltaTime;



		std::ostringstream oss;
		oss << floor(fps);

		fpsCountObj.update(&r, oss.str());


		SDL_SetRenderDrawColor(r.get_SDLRenderer(), 200, 200, 200, 255);
		// Another alex test piece
        o.draw(r.get_SDLRenderer());

        SDL_RenderPresent(r.get_SDLRenderer());

    }


        current_tick = timer.tock();


    }
    printf("%d is last time\n", (int)current_tick);
    /*need to work with a basic input -> update -> render System*/
intMap(map);

	// mapReader::printMap(map);

    return 0;

}