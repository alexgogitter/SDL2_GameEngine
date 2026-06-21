#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <math.h>

// #include "mapReader.h"
#include "fpsCounter.hpp"
#include "render.hpp"
#include "resource_manager.hpp"
#include "time.hpp"
#include "interfaceImplementation.hpp"
#include "object.hpp"
#include "playerObject.hpp"
#include "eventListener.hpp"

//Simple game design follows a Input -> Update -> draw Loop when running


// int mapReader::width, mapReader::height, mapReader::n;


// std::vector<std::vector<colorVals>> map;
int mapWidth, mapHeight;
// std::string mapName = "res/maps/Map2.png";
std::string fontPath = "res/fonts/comicz.ttf";

const int tileSize = 5;
const int mapStartX = 0, mapStartY = 0;

// Frame timing
std::uint64_t frameDeltaMs = 0;

std::uint64_t fpsSampleElapsedMs = 0;
int fpsSampleFrameCount = 0;
float framesPerSecond = 0.0f;

std::vector<Object*> gameObjects;

void demoCallback() {
	ImGui::Begin("Debug Panel");
	ImGui::Text("Engine running");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();
}

int main( int argc, char* argv[] )
{
	// mapReader::Reader(map, mapWidth, mapHeight, mapName);
	Renderer r = Renderer(1080, 1920, 60, IMG_INIT_PNG, "Test Render Window");
	r.Renderer_Init();
	Interface* interface = Interface::create(r.get_SDLWindow(), r.get_SDLRenderer());

	Time frameTimer = Time();

    Resource_manager manager(r.get_SDLRenderer());
    

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;
	EventListener& eventListener = EventListener::Get();

	// Map rendering
	// SDL_Surface* surface = IMG_Load(mapName.c_str());
	// if (surface == nullptr){
	// 	fprintf(stderr, "Surface Load Error");
	// }
	// SDL_Texture* texture = SDL_CreateTextureFromSurface(r.get_SDLRenderer(), surface);
	// if (texture == nullptr){
	// 	fprintf(stderr, "Texture Load Error");
	// }
	// SDL_FreeSurface(surface);
	
	fpsCounter fpsCountObj = fpsCounter();

	interface->addDrawCallback(demoCallback);

	PlayerObject player = PlayerObject(manager, &r);

	player.transform.setPosition(glm::vec2(r.get_screen_height() / 2, r.get_screen_width() / 2));
	player.transform.setScale(glm::vec2(50.0f, 50.0f));
	gameObjects.push_back(&player);

		//While application is running
    while( !quit )
    {  
		frameTimer.tick();

		SDL_RenderClear(r.get_SDLRenderer());

		// if (SDL_RenderCopy(r.get_SDLRenderer(), texture, NULL, mapRect) != 0){
		// 	fprintf(stderr, "RenderCopyError: %s", SDL_GetError());
		// }
        

        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
			eventListener.ProcessEvent(e);
			interface->update(e);

            //User requests quit
            if( e.type == SDL_QUIT )
            {
                r.Renderer_Close();
                quit = true;
            }
        }


		//Scene Drawing goes here.
		for (Object* obj : gameObjects) {
			obj->update(frameDeltaMs);
			obj->draw(&r);
		}
		
		interface->draw(r.get_SDLRenderer());

		std::ostringstream fpsText;
		fpsText << floor(framesPerSecond);
		fpsCountObj.update(&r, fpsText.str());
		
		SDL_SetRenderDrawColor(r.get_SDLRenderer(), 200, 200, 200, 255);
		SDL_RenderPresent(r.get_SDLRenderer());

		frameDeltaMs = frameTimer.tock();
		fpsSampleElapsedMs += frameDeltaMs;
		fpsSampleFrameCount += 1;

		if (fpsSampleFrameCount >= 100) {
			if (fpsSampleElapsedMs > 0) {
				framesPerSecond = (1000.0f * static_cast<float>(fpsSampleFrameCount)) / static_cast<float>(fpsSampleElapsedMs);
			}
			fpsSampleFrameCount = 0;
			fpsSampleElapsedMs = 0;
		}
    }

	//     printf("%d is last time\n", (int)current_tick);
	//     /*need to work with a basic input -> update -> render System*/
	// intMap(map);

	// mapReader::printMap(map);
	    return 0;

}