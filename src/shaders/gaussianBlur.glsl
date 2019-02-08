uniform sampler2D texture0;

uniform bool horizontal;
uniform float weight[8] = float[] (0.197448, 0.174697, 0.120999, 0.065602, 0.02784, 0.009246, 0.002403, 0.000489);

vec3 postProcess(vec2 texCoord) {
    vec2 tex_offset = 1.0 / textureSize(texture0, 0); // gets size of single texel
    vec3 result = texture(texture0, texCoord).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 8; ++i)
        {
            result += texture(texture0, texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(texture0, texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 8; ++i)
        {
            result += texture(texture0, texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(texture0, texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    result = clamp(result, 0, 100);

    return result;
}