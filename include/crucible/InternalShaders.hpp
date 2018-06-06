#pragma once

namespace InternalShaders {

static const std::string debug_vsh = R"(
#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

uniform mat4 view;
uniform mat4 projection;

out vec3 fColor;

void main() {
	gl_Position = projection * view * vec4(vPosition, 1.0);
	fColor = vNormal;
}
)";

static const std::string debug_fsh = R"(
#version 330 core

uniform mat4 view;
uniform mat4 projection;

in vec3 fColor;

out vec4 outColor;

void main() {
    outColor = vec4(fColor, 1.0);
}
)";

static const std::string outline_vsh = R"(
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
	gl_Position = (projection * view * model * vec4(vPosition, 1.0));
    gl_Position = gl_Position + vec4(vNormal * gl_Position.z * thickness, 0.0);
}
)";

static const std::string outline_fsh = R"(
#version 330 core

uniform mat4 view;
uniform mat4 projection;

uniform vec3 color;


out vec4 outColor;

void main() {
    outColor = vec4(color, 1.0);
}
)";

static const std::string shadow_vsh = R"(
#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(vPosition, 1.0f);
}
)";

static const std::string shadow_fsh = R"(
#version 330 core

void main()
{
	// gl_FragDepth = gl_FragCoord.z;
}
)";

static const std::string sprite_vsh = R"(
#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out vec3 fTangent;
out vec2 fTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main() {
	fTexCoord = vTexCoord;
	fTangent = vTangent;
	gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
)";

static const std::string sprite_fsh = R"(
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
)";

static const std::string standard_vsh = R"(
#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;
out mat3 TBN;

void main()
{
    vec4 viewPos = view * model * vec4(vPosition, 1.0);
    fPosition = viewPos.xyz;

    gl_Position = projection * viewPos;

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    fNormal = normalMatrix * normalize(vNormal);

    fTexCoord = vec2(vTexCoord.x, 1-vTexCoord.y);

    // calculate TBN matrix
    vec3 T = normalize(vec3(model * vec4(vTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normalize(vNormal), 0.0)));

    T = normalize(T - dot(T, N) * N); // re-orthogonalize T with respect to N
    vec3 B = cross(N, T); // then retrieve perpendicular vector B with the cross product of T and N

    TBN = mat3(view) * mat3(T, B, N);
}
)";

static const std::string standard_fsh = R"(
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gRoughnessMetallic;

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

uniform bool invertRoughness;
uniform bool roughnessMetallicAlpha;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = fPosition;

	vec4 albedo;
    float roughness;
    float metallic;
    vec3 normal;
    float ao;
    float emission;

    if (albedoTextured) {
    	vec4 texel = texture(albedoTex, fTexCoord);
        albedo = vec4(pow(texel.rgb, vec3(2.2)), texel.a);
    }
    else {
        albedo = vec4(albedoColor, 1.0);
    }

    if (roughnessTextured) {
        roughness = texture(roughnessTex, fTexCoord).r;
    }
    else {
        roughness = roughnessColor;
    }

    if (metallicTextured) {
        metallic = texture(metallicTex, fTexCoord).r;

        if (roughnessMetallicAlpha) {
            roughness = texture(metallicTex, fTexCoord).a;
        }
    }
    else {
        metallic = metallicColor;
    }

    if (normalTextured) {
    	normal = texture(normalTex, fTexCoord).rgb;
    	normal = normalize(normal * 2.0 - 1.0);
    	normal = normalize(TBN * normal);
    }
    else {
        normal = normalize(fNormal);
    }

    if (aoTextured) {
        ao = texture(aoTex, fTexCoord).r;
    }
    else {
        ao = 1.0;
    }

    if (emissionTextured) {
        emission = length(texture(emissionTex, fTexCoord).rgb) * 5;
    }
    else {
        emission = 0.0;
    }

    if (invertRoughness) {
        roughness = 1.0-roughness;
    }

    if (albedo.a > 0.5) {
        gNormal = normalize(normal);
        gAlbedo = albedo;
        gRoughnessMetallic = vec4(roughness, metallic, ao, emission);
    }
    else {
        discard;
    }
}
)";

static const std::string postProcessing_vsh = R"(
#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out vec2 fTexCoord;

void main()
{
    fTexCoord = vTexCoord;
    gl_Position = vec4(vPosition, 1.0);
}
)";

static const std::string ssaoBlur_glsl = R"(
uniform sampler2D ssaoInput;

vec3 postProcess(vec2 texCoord) {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, texCoord + offset).r;
        }
    }
    return vec3(result / (4.0 * 4.0));
}
)";

static const std::string ssao_glsl = R"(
#include <lighting>

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int kernelSize;

uniform vec3 samples[256];

uniform mat4 projection;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)

uniform float radius = 10.5;
float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0);

vec3 postProcess(vec2 texCoord) {
// get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, texCoord).xyz;
    vec3 normal = normalize(texture(gNormal, texCoord).rgb);
    vec3 randomVec = normalize(texture(texNoise, texCoord * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // from tangent to view-space
        sample = fragPos + sample * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0


        if (length(texture(gPosition, offset.xy).rgb) > 0.0) {
            // get sample depth
            float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample


            // range check & accumulate
            float rangeCheck = smoothstep(0.0, 1.0, radius / abs((fragPos.z - sampleDepth) * 10));
            occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
        }
    }
    occlusion = 1.0 - (occlusion / kernelSize);



    if (length(fragPos) > 0.0) {
        return vec3(occlusion);
	}
	else {
		discard;
	}
}
)";

static const std::string deferred_glsl = R"(
#include <lighting>

#define MAX_POINT_LIGHTS 100

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughnessMetallic;
uniform samplerCube irradiance;
uniform samplerCube prefilter;
uniform sampler2D brdf;
uniform sampler2D ssaoTex;


uniform bool ssaoEnabled;
uniform bool doIBL;

uniform sampler2DShadow shadowTextures[4];
uniform vec3 ambient;
uniform vec3 cameraPos;
uniform float cascadeDistances[4];

uniform int pointLightCount;
uniform PointLight[MAX_POINT_LIGHTS] pointLights;

uniform mat4 lightSpaceMatrix[4];
uniform mat4 view;

uniform float bloomStrength;

uniform DirectionalLight sun;

const float PI = 3.14159265359;

float inShadow(vec4 shadowspace) {
    float bias = 0.001;
    if (shadowspace.x > 0.0+bias && shadowspace.x < 1.0-bias) {
        if (shadowspace.y > 0.0+bias && shadowspace.y < 1.0-bias) {
            if (shadowspace.z > 0.0+bias && shadowspace.z < 1.0-bias) {
                return 1.0;
            }
        }
    }
    return 0.0;
}

vec3 lighting(vec3 fragPos, vec3 albedo, vec3 normal, float roughness, float metallic, float ao) {
	vec3 N = normalize(normal);
	vec3 V = normalize(cameraPos - fragPos);

	vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    //CSM
	vec4 fragLightSpace0 = biasMatrix * lightSpaceMatrix[0] * inverse(view) * vec4(fragPos, 1.0);
    vec4 fragLightSpace1 = biasMatrix * lightSpaceMatrix[1] * inverse(view) * vec4(fragPos, 1.0);
    vec4 fragLightSpace2 = biasMatrix * lightSpaceMatrix[2] * inverse(view) * vec4(fragPos, 1.0);
    vec4 fragLightSpace3 = biasMatrix * lightSpaceMatrix[3] * inverse(view) * vec4(fragPos, 1.0);

    vec3 ls0 = vec3(fragLightSpace0.xy, (fragLightSpace0.z)/fragLightSpace0.w);

    float distance = length(cameraPos - fragPos);
	float shadow0 = mix(1.0, ShadowCalculationPCF(0.0005, fragLightSpace0, shadowTextures[0]), inShadow(fragLightSpace0));
    float shadow1 = mix(1.0, ShadowCalculation(0.0005, fragLightSpace1, shadowTextures[1]), inShadow(fragLightSpace1) * (1-inShadow(fragLightSpace0)));
    float shadow2 = mix(1.0, ShadowCalculation(0.0005, fragLightSpace2, shadowTextures[2]), inShadow(fragLightSpace2) * (1-inShadow(fragLightSpace1)));
    float shadow3 = mix(1.0, ShadowCalculation(0.0005, fragLightSpace3, shadowTextures[3]), inShadow(fragLightSpace3) * (1-inShadow(fragLightSpace2)));
    float shadow = shadow0 * shadow1 * shadow2 * shadow3;
	//float shadow = 1.0;
	//float shadow = ShadowCalculation(0.1 / cascadeDistances[0], fragLightSpace0, shadowTextures[0]);

	//point lights
	for (int i = 0; i < pointLightCount; i++) {
		if (i < MAX_POINT_LIGHTS) {
			PointLight light = pointLights[i];

            // calculate per-light radiance
            vec3 L = normalize(light.position - fragPos);
            vec3 H = normalize(V + L);

            float distance    = length(light.position - fragPos);
            float attenuation = 1.0 / (distance * distance);

            vec3 radiance     = light.color * attenuation;

            // cook-torrance brdf
            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);
            vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            vec3 nominator    = NDF * G * F;
            float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
            vec3 specular     = nominator / denominator;

            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);
            if (attenuation > 0.0001) {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            }
		}
	}

	//directional light
	{
		DirectionalLight light = sun;

		// calculate per-light radiance
        vec3 L = normalize(-sun.direction);
        vec3 H = normalize(V + L);
        vec3 radiance     = light.color;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlickRoughness(clamp(dot(H, V), 0.0, 1.0), F0, roughness);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = nominator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadow;
        //Lo += kD;
	}

	vec3 F        = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradianceColor = texture(irradiance, normalize(mat3(inverse(view)) * N)).rgb;
	vec3 diffuse = irradianceColor * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilter, normalize(mat3(inverse(view)) * R),  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdfColor = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;

	vec3 specular = prefilteredColor * (F * brdfColor.x + brdfColor.y);

    vec3 ambientColor;
    if (doIBL) {
	    ambientColor = (kD * diffuse + specular);
    }
    else {
        ambientColor = ambient * albedo;
    }

    vec3 color = (ambientColor + Lo) * ao;
    //color = ambient * ao;


    //color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0/2.2));

    return Lo;
}

vec3 postProcess(vec2 texCoord) {

    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, texCoord).rgb;
    vec3 Normal = texture(gNormal, texCoord).rgb;
    vec3 Albedo = texture(gAlbedo, texCoord).rgb;
	vec4 RoughnessMetallic = texture(gRoughnessMetallic, texCoord);
	float Roughness = RoughnessMetallic.r;
	float Metallic = RoughnessMetallic.g;
    float emission = RoughnessMetallic.a;
    float ao = RoughnessMetallic.b;

    float ssao = 1.0;
    if (ssaoEnabled) {
        ssao = texture(ssaoTex, texCoord).r;
    }

    vec3 light = lighting(FragPos, Albedo, Normal, Roughness, Metallic, ssao);


    vec4 BrightColor;
    float brightness = dot(light, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.9)
        BrightColor = vec4(light, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    BrightColor = vec4((bloomStrength * light) + (emission * Albedo), 1.0);

    outColor1 = BrightColor; // TODO: very hacky

	if (length(FragPos) > 0.0) {
		return light + (emission * Albedo);
        //return vec3(ssao);
	}
	else {
		discard;
	}
}
)";

static const std::string tonemap_glsl = R"(
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

uniform bool vignette;
uniform bool tonemap;
uniform bool bloom;

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
    vec3 color;
    if (bloom) {
        color = (texture(texture0, texCoord).rgb) + (texture(texture1, texCoord).rgb) + (texture(texture2, texCoord).rgb) + texture(texture3, texCoord).rgb;
    }
    else {
        color = texture(texture3, texCoord).rgb;
    }

    vec3 grayscale = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
    vec2 texelSize = 1.0 / textureSize(texture0, 0).xy;


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
        const float power = 0.1;
        vec2 tuv = fTexCoord * (vec2(1.0) - texCoord.yx);
        float vign = tuv.x*tuv.y * strength;
        vign = pow(vign, power);
        color *= vign;
    }

	return color;
}
)";

static const std::string fxaa_glsl = R"(
uniform sampler2D texture0;

vec3 postProcess(vec2 texCoord) {

    vec2 frameBufSize = textureSize(texture0, 0).xy;

    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0/8.0;
    float FXAA_REDUCE_MIN = 1.0/128.0;

    vec3 rgbNW=texture2D(texture0,texCoord+(vec2(-1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbNE=texture2D(texture0,texCoord+(vec2(1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbSW=texture2D(texture0,texCoord+(vec2(-1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbSE=texture2D(texture0,texCoord+(vec2(1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbM=texture2D(texture0, texCoord).xyz;

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / frameBufSize;

    vec3 rgbA = (1.0/2.0) * (
        texture2D(texture0, texCoord + dir * (1.0/3.0 - 0.5)).xyz +
        texture2D(texture0, texCoord + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture2D(texture0, texCoord + dir * (0.0/3.0 - 0.5)).xyz +
        texture2D(texture0, texCoord + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)){
        return rgbA;
    }else{
        return rgbB;
    }
}
)";

static const std::string gaussianBlur_glsl = R"(
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
)";

static const std::string passthrough_glsl = R"(
uniform sampler2D texture0;

vec3 postProcess(vec2 texCoord) {
    return texture(texture0, texCoord).rgb;
}
)";

static const std::string eq2cube_fsh = R"(
#version 330 core

out vec4 outColor;

in vec3 fPosition;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(fPosition)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;

    outColor = vec4(color, 1.0);
}

)";

static const std::string cubemap_vsh = R"(
#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out vec3 fPosition;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    fPosition = vPosition;

	mat4 newView = mat4(mat3(view));

    gl_Position =  projection * newView * vec4(fPosition, 1.0);
}
)";

static const std::string cubemap_fsh = R"(
#version 330 core

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outColor1;

in vec3 fPosition;

uniform samplerCube environmentMap;
uniform vec3 ambient;
uniform bool isTextured;
uniform float bloomStrength;

void main()
{
    vec3 envColor;
    if (isTextured)
        envColor = texture(environmentMap, fPosition).rgb;
    else
        envColor = ambient;

    outColor = vec4(envColor, 1.0);
    outColor1 = vec4(envColor * bloomStrength, 1.0);
}
)";

static const std::string irradiance_fsh = R"(
#version 330 core

out vec4 outColor;

in vec3 fPosition;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    // the sample direction equals the hemisphere's orientation
    vec3 N = normalize(fPosition);

	vec3 irradiance = vec3(0.0);

	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up         = cross(N, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outColor = vec4(irradiance, 1.0);
}
)";

static const std::string prefilter_fsh = R"(
#version 330 core

out vec4 outColor;

in vec3 fPosition;

uniform samplerCube environmentMap;
uniform float roughness;

#include <lighting>

void main()
{
    vec3 N = normalize(fPosition);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 2048u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}

)";

static const std::string brdf_glsl = R"(
uniform sampler2D texture0;

#include <lighting>

vec3 postProcess(vec2 texCoord) {
    vec2 integratedBRDF = IntegrateBRDF(texCoord.x, texCoord.y);
    return vec3(integratedBRDF, 0.0);
}
)";


//library shaders
static const std::string lighting_glsl = R"(

#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

struct PointLight {
vec3 position;
vec3 color;
};

struct DirectionalLight {
vec3 direction;
vec3 color;
};

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
float a      = roughness*roughness;
float a2     = a*a;
float NdotH  = max(dot(N, H), 0.0);
float NdotH2 = NdotH*NdotH;

float nom   = a2;
float denom = (NdotH2 * (a2 - 1.0) + 1.0);
denom = 3.141592 * denom * denom;

return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
float r = (roughness + 1.0);
float k = (r*r) / 8.0;

float nom   = NdotV;
float denom = NdotV * (1.0 - k) + k;

return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
float NdotV = max(dot(N, V), 0.0);
float NdotL = max(dot(N, L), 0.0);
float ggx2  = GeometrySchlickGGX(NdotV, roughness);
float ggx1  = GeometrySchlickGGX(NdotL, roughness);

return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits)
{
 bits = (bits << 16u) | (bits >> 16u);
 bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
 bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
 bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
 bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
 return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
float a = roughness*roughness;

float phi = 2.0 * 3.141592 * Xi.x;
float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

// from spherical coordinates to cartesian coordinates - halfway vector
vec3 H;
H.x = cos(phi) * sinTheta;
H.y = sin(phi) * sinTheta;
H.z = cosTheta;

// from tangent-space H vector to world-space sample vector
vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
vec3 tangent   = normalize(cross(up, N));
vec3 bitangent = cross(N, tangent);

vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
vec2 IntegrateBRDF(float NdotV, float roughness)
{
vec3 V;
V.x = sqrt(1.0 - NdotV*NdotV);
V.y = 0.0;
V.z = NdotV;

float A = 0.0;
float B = 0.0;

vec3 N = vec3(0.0, 0.0, 1.0);

const uint SAMPLE_COUNT = 1024u;
for(uint i = 0u; i < SAMPLE_COUNT; ++i)
{
    // generates a sample vector that's biased towards the
    // preferred alignment direction (importance sampling).
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(L.z, 0.0);
    float NdotH = max(H.z, 0.0);
    float VdotH = max(dot(V, H), 0.0);

    if(NdotL > 0.0)
    {
        float G = GeometrySmith(N, V, L, roughness);
        float G_Vis = (G * VdotH) / (NdotH * NdotV);
        float Fc = pow(1.0 - VdotH, 5.0);

        A += (1.0 - Fc) * G_Vis;
        B += Fc * G_Vis;
    }
}
A /= float(SAMPLE_COUNT);
B /= float(SAMPLE_COUNT);
return vec2(A, B);
}
// ----------------------------------------------------------------------------
const mat4 biasMatrix = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
);

const vec2 poissonDisk[16] = vec2[](
vec2( -0.94201624, -0.39906216 ),
vec2( 0.94558609, -0.76890725 ),
vec2( -0.094184101, -0.92938870 ),
vec2( 0.34495938, 0.29387760 ),
vec2( -0.91588581, 0.45771432 ),
vec2( -0.81544232, -0.87912464 ),
vec2( -0.38277543, 0.27676845 ),
vec2( 0.97484398, 0.75648379 ),
vec2( 0.44323325, -0.97511554 ),
vec2( 0.53742981, -0.47373420 ),
vec2( -0.26496911, -0.41893023 ),
vec2( 0.79197514, 0.19090188 ),
vec2( -0.24188840, 0.99706507 ),
vec2( -0.81409955, 0.91437590 ),
vec2( 0.19984126, 0.78641367 ),
vec2( 0.14383161, -0.14100790 )
);
// ----------------------------------------------------------------------------
float ShadowCalculationPCF(float bias, vec4 fragPosLightSpace, sampler2DShadow shadowMap)
{
float visibility = 1.0;

for (int i=0;i<8;i++){
    int index = i;
    visibility -= (1.0/8.0)*(1.0-texture( shadowMap, vec3(fragPosLightSpace.xy + poissonDisk[index]/1000.0, (fragPosLightSpace.z)/fragPosLightSpace.w-bias) ));
}
return clamp(visibility, 0, 1);
}
// ----------------------------------------------------------------------------
float ShadowCalculation(float bias, vec4 fragPosLightSpace, sampler2DShadow shadowMap)
{
float visibility = 1.0;

visibility -= (1.0-texture( shadowMap, vec3(fragPosLightSpace.xy, (fragPosLightSpace.z)/fragPosLightSpace.w-bias) ));

return clamp(visibility, 0, 1);
}
#endif
)";
}
