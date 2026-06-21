#include "render.hpp"
#include <string>
#include <SDL_image.h>

Renderer::Renderer(int s_w, int s_h, int fc, int i_f, const char* title)
{
    this->SCREEN_WIDTH=s_w;
    this->SCREEN_HEIGHT=s_h;
    this->FRAME_CAP=fc;
    this->IMAGE_FLAGS=i_f;

    this->sTitle=title;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Unable to Initialize SDL! SDL_ERROR: %s\n", SDL_GetError());
        this->init_success=false;
    }
    else
    {
        if((IMG_Init(IMAGE_FLAGS) != IMAGE_FLAGS))
        {
            fprintf(stderr, "Unable to initialize SDL2_image with flags: %d. SDL_ERROR: %s\n", IMAGE_FLAGS, SDL_GetError());
            this->init_success=false;
        }

        if((TTF_Init() != 0))
        {
            fprintf(stderr, "Unable to initialize SDL_ttf!");
            this->init_success=false;
        }

    }
    
}

Renderer::~Renderer()
{
    gWindow = NULL;
    gRenderer = NULL;
    camera=NULL;

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

int Renderer::Renderer_Init()
{
    //return 0 on success of Init, else return NOTZERO.
    int status=0;
    gWindow = SDL_CreateWindow(this->sTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_HEIGHT, SCREEN_WIDTH, SDL_WINDOW_SHOWN);
    // Sets wether the window is resizeable
    SDL_SetWindowResizable(gWindow, SDL_TRUE);
    if(gWindow==NULL)
    {
        printf( "Window could not be created! SDL_ERROR: %s\n", SDL_GetError() );
        status=1;
    }
    else
    {
        //Create Renderer for window
        gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC ); // SDL_RENDERER_PRESENTVSYNC 
        if( gRenderer == NULL )
        {
            fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
            status=2;
        }
        else
        {
            //Initialize Renderer color
            SDL_SetRenderDrawColor( gRenderer, 0x2E, 0x8B, 0x57, 0xFF );
            //Initialize PNG loading
            int imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                status=3;
            }
        }
    }

    return status;

}

void Renderer::Renderer_Close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
}

// _text : string to show; font if path to font; _src section of texture; _dest where and how to display; textcolor - self explanitory
void Renderer::Renderer_ttf(std::string _text, TTF_Font* _font, SDL_Rect *_src, SDL_Rect *_dest, SDL_Color &_textColor){

    if (_font == NULL){
        fprintf(stderr, "FONT NOT LOADED Renderer::Renderer_ttf %s\n", TTF_GetError());
    }

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(_font, _text.c_str(), _textColor);

    SDL_Texture* Message = SDL_CreateTextureFromSurface(this->gRenderer, surfaceMessage);

    SDL_Rect TextContainer = {0,0,100,50};

    SDL_RenderCopy(this->gRenderer, Message, _src, _dest);

    // Free resorces
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

