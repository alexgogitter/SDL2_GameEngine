#include "fpsCounter.hpp"



TTF_Font* fpsCounter::loadTTFFont(std::string* filePath){
	TTF_Font* Sans = TTF_OpenFont(filePath->c_str(), 8);
    return Sans;
}

fpsCounter::fpsCounter()
{
    this->fpsFont = TTF_OpenFont(fontPath.c_str(), fontSize);
}

fpsCounter::fpsCounter(std::string* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor) : fpsColor(new SDL_Color{0, 128, 47, 255})
{
    this->fpsFont = loadTTFFont(_fpsFont);
    this->src = _src;
    this->dest = _dest;
    this->fpsColor = _fpsColor;
}

fpsCounter::fpsCounter(TTF_Font* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor) 
{
    this->fpsFont = _fpsFont;
    this->src = _src;
    this->dest = _dest;
    this->fpsColor = _fpsColor;
}


fpsCounter::~fpsCounter()
{

    TTF_CloseFont(fpsFont);

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
void fpsCounter::update(Renderer* r, std::string text)
{
    r->Renderer_ttf(text, fpsFont, src, dest, *fpsColor);
}

// static void setDeltaTime(std::uint64_t _deltaTime){
//     deltaTime = _deltaTime;
// }