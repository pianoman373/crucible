#version 330 core

layout (location = 0) out vec4 outColor;

in vec3 fPosition;

uniform samplerCube environmentMap;
uniform vec3 ambient;
uniform bool isTextured;

void main()
{
    vec3 envColor = texture(environmentMap, fPosition).rgb;

    outColor = vec4(envColor, 1.0);
}
