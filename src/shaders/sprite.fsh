#version 330 core

uniform sampler2D sprite;
uniform vec3 cameraPos;
uniform vec4 uvOffsets;

in vec3 fTangent;
in vec2 fTexCoord;

out vec4 outColor;

void main() {
	outColor = texture(sprite, (fTexCoord * uvOffsets.zw) + uvOffsets.xy);

    if (outColor.w < 0.01) {
        discard;
    }
}