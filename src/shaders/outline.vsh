#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float thickness;

void main() {
	gl_Position = gl_Position + vec4(vNormal * gl_Position.z * thickness, 0.0);
}