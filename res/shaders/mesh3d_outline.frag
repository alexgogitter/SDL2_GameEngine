#version 330 core

out vec4 fragmentColour;

uniform vec4 uOutlineColour;

void main()
{
    fragmentColour = uOutlineColour;
}
