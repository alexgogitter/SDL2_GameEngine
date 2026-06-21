#ifndef TIME_H
#define TIME_H

#include <cstdint>
#include <SDL.h>

class Time
{
 private:
   std::uint64_t time_elapsed = 0;
   std::uint64_t time_last_tick = 0;
   std::uint64_t time_update_time = SDL_MAX_UINT64;
 public:
    
    //used to mark the start of an update
    void tick();
    
    //used for measuring duration of update
  std::uint64_t tock();

    //Returns the time since last tick
  std::uint64_t getElapsed();
};

#endif // TIME_H