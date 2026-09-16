#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinate;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vTextureCoordinate;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);

    vWorldPosition = worldPosition.xyz;

    vWorldNormal = normalize(mat3(transpose(inverse(uModel))) * aNormal);

    vTextureCoordinate = aTextureCoordinate;

    gl_Position = uViewProjection * worldPosition;
}