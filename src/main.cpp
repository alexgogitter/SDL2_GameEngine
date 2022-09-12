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



        SDL_RenderPresent(r.get_SDLRenderer());

        
    }

    /*need to work with a basic input -> update -> render System*/


    return 0;

}