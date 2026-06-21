#pragma once
#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include <cstdint>
#include <string>
#include <stdio.h>

class Object;

class Component
{
    protected:
        Object* parent;
    private:
        
        virtual void Update(std::uint64_t deltaTime) = 0;
    public:
        std::string name;
        Component(const std::string& _name, Object* _parent): name(_name), parent(_parent) {}
        void update(std::uint64_t deltaTime);
};


#endif // !COMPONENT_HPP