#ifndef PLAYEROBJECT_HPP
#define PLAYEROBJECT_HPP
#include "object.hpp"

class PlayerObject : public Object {
    private:   

    public:
        PlayerObject(Resource_manager& r, Renderer* rend) : Object("Player", r, rend) {
            // Initialize player-specific properties here
            this->add_Component(new ColliderComponent(this, SDL_Rect{0, 0, 100, 100}));
        }

        void Update(std::uint64_t deltaTime) override {
            // Implement player-specific update logic here
        }

};


#endif // PLAYEROBJECT_HPP