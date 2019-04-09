uniform sampler2D tex;

vec3 postProcess(vec2 texCoord) {
    vec3 color = texture(tex, texCoord).rgb;

	return 1 - color;
}