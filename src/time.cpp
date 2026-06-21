#include "time.hpp"
/*Sets the current time at the start of an update*/
void Time::tick()
{
    time_last_tick = SDL_GetTicks64();
}

/*returns the time since last tick for update duration.*/
std::uint64_t Time::tock()
{
    time_update_time = SDL_GetTicks64() - time_last_tick;
    return time_update_time;
}

std::uint64_t Time::getElapsed()
{
    return SDL_GetTicks64() - time_last_tick;
}
