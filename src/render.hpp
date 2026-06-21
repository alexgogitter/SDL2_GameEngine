#ifndef RENDER_HPP
#define RENDER_HPP

#include <SDL_ttf.h>
#include <string>



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

    int get_screen_width() { return SCREEN_WIDTH; }
    int get_screen_height() { return SCREEN_HEIGHT; }
    

    int Renderer_Init();

    void Renderer_Close();

    SDL_Renderer* get_SDLRenderer(){return gRenderer;}
    SDL_Window* get_SDLWindow(){return gWindow;}

    void Renderer_PresentFrame(){SDL_RenderPresent(gRenderer);}

    void Renderer_ttf(std::string _text, TTF_Font* _font, SDL_Rect *_src, SDL_Rect *_dest, SDL_Color &_textColor);


};

#endif // RENDER_HPP