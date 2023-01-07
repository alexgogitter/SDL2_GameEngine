#include "object.h"
#include <SDL_ttf.h>



class Renderer
{
private:

    bool init_success = false;
    SDL_Window* gWindow = NULL;
    SDL_Renderer* gRenderer = NULL;
    SDL_DisplayMode* DM;
    SDL_Rect* camera;
    const char* sTitle;
    int SCREEN_WIDTH, SCREEN_HEIGHT, FRAME_CAP, IMAGE_FLAGS, DISPLAY_NUMBER;

public:
    //default constructor
    Renderer();
    //initializes all the SDL modules that might be needed to run a game
    Renderer(int s_w, int s_h, int fc, int i_f, const char* title);
    //closes all the SDL modules opened by the constructor
    ~Renderer();

    int Renderer_Init();

    void Renderer_Close();

    void Renderer_Draw(Object* o);

    SDL_Renderer* get_SDLRenderer(){return gRenderer;}

    void Renderer_PresentFrame(){SDL_RenderPresent(gRenderer);}

};