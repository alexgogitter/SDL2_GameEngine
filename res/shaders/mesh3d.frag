#version 330 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vTextureCoordinate;

out vec4 fragmentColour;

uniform vec4 uColour;

void main()
{
    vec3 normal = normalize(vWorldNormal);

    vec3 lightDirection = normalize(vec3(-0.35, 0.85, 0.45));

    float diffuse = max(dot(normal, lightDirection), 0.0);

    float lighting = 0.18 + diffuse * 0.82;

    fragmentColour = vec4(uColour.rgb * lighting, uColour.a);
}