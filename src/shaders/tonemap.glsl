uniform sampler2D bloom0;
uniform sampler2D bloom1;
uniform sampler2D bloom2;
uniform sampler2D deferred;
uniform sampler2D ssaoTexture;

uniform bool vignette;
uniform bool tonemap;
uniform bool bloom;
uniform bool ssao;

uniform float bloomStrength;

vec3 aces(vec3 col, float exposure)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((col*(a*col+b))/(col*(c*col+d)+e), 0.0, 1.0);
}

vec3 reinhard(vec3 col, float exposure) {
    return vec3(1.0) - exp(-col * exposure);
}

vec3 postProcess(vec2 texCoord) {
    vec3 color = texture(deferred, texCoord).rgb;

    if (ssao) {
        color *= texture(ssaoTexture, texCoord).rgb;
    }

    if (bloom) {
        color += (texture(bloom0, texCoord).rgb*bloomStrength) + (texture(bloom1, texCoord).rgb*bloomStrength) + (texture(bloom2, texCoord).rgb*bloomStrength);
    }

    // HDR tonemapping
    if (tonemap) {
        const float exposure = 1.0;
        color = aces(color, exposure);
        // gamma correct
        color = pow(color, vec3(1.0/2.2));
    }

    // vignette
    if (vignette)
    {
        const float strength = 10.0;
        const float power = 0.2;
        vec2 tuv = fTexCoord * (vec2(1.0) - texCoord.yx);
        float vign = tuv.x*tuv.y * strength;
        vign = pow(vign, power);
        color *= vign;
    }

	return color;
}