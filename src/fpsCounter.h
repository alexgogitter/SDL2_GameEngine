#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

// To access the renderer
#include "render.h"

class fpsCounter {
private:
    SDL_Renderer* gRenderer = NULL;
    TTF_Font* fpsFont;

    // making these ptrs faster?
    SDL_Rect* src;
    SDL_Rect* dest;

    SDL_Color* fpsColor;

    int width = 100, height = 50;
    int xPos = 0, yPos = 0;



public:
    // Default constructor
    fpsCounter();

    fpsCounter(TTF_Font* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor);

    // Default destructor
    ~fpsCounter();

    void update(Renderer* r, std::string text);

};


#endif // FPSCOUNTER_H