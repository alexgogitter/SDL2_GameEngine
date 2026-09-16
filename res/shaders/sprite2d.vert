#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTextureCoordinate;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec2 vTextureCoordinate;
out vec2 vWorldPosition;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 0.0, 1.0);
    vWorldPosition = worldPosition.xy;
    vTextureCoordinate = aTextureCoordinate;
    gl_Position = uViewProjection * worldPosition;
}
