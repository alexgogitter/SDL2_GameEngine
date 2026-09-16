#version 330 core

#define MAX_POINT_LIGHTS_2D 16

struct PointLight2D
{
    vec3 position;
    vec3 colour;
    float intensity;
    float radius;
};

in vec2 vTextureCoordinate;
in vec2 vWorldPosition;

out vec4 fragmentColour;

uniform sampler2D uAlbedoTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uHeightTexture;
uniform sampler2D uEmissionTexture;
uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform sampler2D uAlphaMaskTexture;

uniform bool uHasAlbedo;
uniform bool uHasAlphaMask;
uniform bool uHasDiffuse;
uniform bool uHasNormal;
uniform bool uHasHeight;
uniform bool uHasEmission;
uniform bool uHasSpecular;
uniform bool uLit;

uniform vec4 uTint;
uniform float uSpriteRotation;
uniform float uNormalStrength;
uniform float uHeightScale;
uniform float uSpecularStrength;
uniform float uShininess;

uniform vec4 uAlbedoUvRect;
uniform vec4 uAlphaMaskUvRect;
uniform vec4 uDiffuseUvRect;
uniform vec4 uNormalUvRect;
uniform vec4 uHeightUvRect;
uniform vec4 uEmissionUvRect;
uniform vec4 uSpecularUvRect;

uniform vec3 uAmbientLight;
uniform int uLightCount;
uniform PointLight2D uLights[MAX_POINT_LIGHTS_2D];

vec2 remapUv(vec4 uvRect)
{
    return uvRect.xy + vTextureCoordinate * uvRect.zw;
}

vec3 sampleWorldNormal()
{
    if (!uHasNormal)
    {
        return vec3(0.0, 0.0, 1.0);
    }

    vec3 normal = texture(uNormalTexture, remapUv(uNormalUvRect)).rgb * 2.0 - 1.0;
    normal.xy *= uNormalStrength;

    float cosine = cos(uSpriteRotation);
    float sine = sin(uSpriteRotation);
    normal.xy = mat2(cosine, sine, -sine, cosine) * normal.xy;
    return normalize(normal);
}

void main()
{
    vec4 albedo = uHasAlbedo
        ? texture(uAlbedoTexture, remapUv(uAlbedoUvRect)) * uTint
        : uTint;
    albedo.a *= uHasAlphaMask
        ? texture(uAlphaMaskTexture, remapUv(uAlphaMaskUvRect)).r
        : 1.0;

    if (albedo.a < 0.01)
    {
        discard;
    }

    vec3 emission = uHasEmission
        ? texture(uEmissionTexture, remapUv(uEmissionUvRect)).rgb
        : vec3(0.0);

    if (!uLit)
    {
        fragmentColour = vec4(albedo.rgb + emission, albedo.a);
        return;
    }

    float surfaceHeight = uHasHeight
        ? texture(uHeightTexture, remapUv(uHeightUvRect)).r * uHeightScale
        : 0.0;
    vec3 fragmentPosition = vec3(vWorldPosition, surfaceHeight);
    vec3 normal = sampleWorldNormal();
    vec3 accumulatedLight = uAmbientLight;
    vec3 accumulatedSpecular = vec3(0.0);
    float roughness = uHasDiffuse
        ? clamp(texture(uDiffuseTexture, remapUv(uDiffuseUvRect)).r, 0.0, 1.0)
        : 1.0;
    float matteResponse = uHasDiffuse
        ? mix(0.35, 1.0, roughness)
        : 1.0;
    float specularRoughnessDamping = uHasDiffuse
        ? mix(1.0, 0.25, roughness)
        : 1.0;
    vec3 specularMap = uHasSpecular
        ? texture(uSpecularTexture, remapUv(uSpecularUvRect)).rgb
        : vec3(0.0);
    float specularGloss = clamp(dot(specularMap, vec3(0.2126, 0.7152, 0.0722)), 0.0, 1.0);
    vec3 specularColour = specularMap * uSpecularStrength;
    float specularExponent = mix(4.0, max(uShininess, 1.0), specularGloss);
    vec3 viewDirection = vec3(0.0, 0.0, 1.0);

    for (int index = 0; index < uLightCount; ++index)
    {
        vec3 difference = uLights[index].position - fragmentPosition;
        vec3 lightDirection = normalize(difference);
        float planarDistance = length(difference.xy);
        float attenuation = clamp(
            1.0 - planarDistance / max(uLights[index].radius, 0.001),
            0.0,
            1.0
        );
        attenuation *= attenuation;

        vec3 radiance = uLights[index].colour * uLights[index].intensity * attenuation;
        float diffuse = max(dot(normal, lightDirection), 0.0);
        accumulatedLight += radiance * diffuse * matteResponse;

        if (uHasSpecular && specularGloss > 0.0 && diffuse > 0.0)
        {
            vec3 halfDirection = normalize(lightDirection + viewDirection);
            float specular = pow(
                max(dot(normal, halfDirection), 0.0),
                specularExponent
            );
            accumulatedSpecular += radiance *
                specularColour *
                specularRoughnessDamping *
                specular;
        }
    }

    fragmentColour = vec4(
        albedo.rgb * accumulatedLight + accumulatedSpecular + emission,
        albedo.a
    );
}
