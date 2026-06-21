#ifndef FPSCOUNTER_HPP
#define FPSCOUNTER_HPP

#include <cstdint>
// To access the renderer
#include "render.hpp"



class fpsCounter {
private:

    // static std::uint64_t deltaTime;
    SDL_Renderer* gRenderer = NULL;
    TTF_Font* fpsFont;

    // making these ptrs faster?
    SDL_Rect* src = NULL;
    SDL_Rect* dest = new SDL_Rect{0,0,100,50};
    SDL_Color* fpsColor = new SDL_Color{0, 128, 47, 255};

    std::string fontPath = "res/fonts/comicz.ttf";
    int fontSize = 8;

    int width = 100, height = 50;
    int xPos = 0, yPos = 0;

    int fpsSampleSize;
    int fpsMSTotal;

    TTF_Font* loadTTFFont(std::string* filePath);

public:
    // Default constructor
    fpsCounter();

    fpsCounter(std::string* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor);

    fpsCounter(TTF_Font* _fpsFont, SDL_Rect* _src, SDL_Rect* _dest, SDL_Color* _fpsColor);

    // Default destructor
    ~fpsCounter();

    void update(Renderer* r, std::string text);

    static void setDeltaTime(std::uint64_t _deltaTime);

};


#endif // FPSCOUNTER_HPP