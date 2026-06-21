#include "colliderComponent.hpp"



void ColliderComponent::Update(std::uint64_t deltaTime)
{
    // Implement any necessary update logic for the collider component here
    EventListener& eventListener = EventListener::Get();
    const EventListener::MouseState& mouseState = eventListener.GetMouseState();
    if(mouseState.x >= collider_box.x + parent->transform.getPosition()[0] - collider_box.w/2 && mouseState.x <= collider_box.x + parent->transform.getPosition()[0] + collider_box.w/2 &&
       mouseState.y >= collider_box.y + parent->transform.getPosition()[1] - collider_box.h/2 && mouseState.y <= collider_box.y + parent->transform.getPosition()[1] + collider_box.h/2)
    {
        // Mouse is over the collider box
        // You can add logic here to handle mouse-over events
        parent->draw_colour = glm::vec4(0.0f, 255.0f, 0.0f, 255.0f); // Change color when mouse is over
    }
    else
    {
        // Mouse is not over the collider box
        parent->draw_colour = glm::vec4(255.0f, 0.0f, 0.0f, 255.0f); // Change color back to red when mouse is not over
    }

}