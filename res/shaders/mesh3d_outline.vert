#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinate;

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform float uOutlineScale;

void main()
{
    vec3 expandedPosition = aPosition * uOutlineScale;

    gl_Position = uViewProjection * uModel * vec4(expandedPosition, 1.0);
}
