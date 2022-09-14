#include "fpsCounter.h"

fpsCounter::fpsCounter(){}

fpsCounter::fpsCounter(TTF_Font* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor){
    this->fpsFont = _fpsFont;
    this->src = _src;
    this->dest = _dest;
    this->fpsColor = _fpsColor;
}


fpsCounter::~fpsCounter(){
    gRenderer = NULL;
    fpsFont = NULL;
    src = NULL;
    dest = NULL;
    fpsColor = NULL;
}

// void fpsCounter::draw(Renderer* r){
//     r->Renderer_ttf(std::string _text, TTF_Font* _font, SDL_Rect *_src, SDL_Rect *_dest, SDL_Color &_textColor);
// }

// To be called each frame
void fpsCounter::update(Renderer* r, std::string text){
    r->Renderer_ttf(text, fpsFont, src, dest, *fpsColor);
}