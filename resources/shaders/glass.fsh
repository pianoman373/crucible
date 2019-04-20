#version 330 core
layout (location = 0) out vec4 outColor;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;
in mat3 TBN;

uniform sampler2D albedoTex;
uniform bool albedoTextured;
uniform vec3 albedoColor;

uniform sampler2D roughnessTex;
uniform bool roughnessTextured;
uniform float roughnessColor;

uniform sampler2D metallicTex;
uniform bool metallicTextured;
uniform float metallicColor;

uniform sampler2D normalTex;
uniform bool normalTextured;

uniform sampler2D aoTex;
uniform bool aoTextured;

uniform sampler2D emissionTex;
uniform bool emissionTextured;
uniform float emission;

uniform bool invertRoughness;
uniform bool roughnessMetallicAlpha;

void main()
{
    outColor = vec4(1.0, 0.0, 0.0, 0.2);
}