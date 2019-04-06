#include <lighting>

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughnessMetallic;

uniform sampler2D shadowTextures[4];
uniform float cascadeDistances[4];
uniform mat4 lightSpaceMatrix[4];

uniform mat4 view;

uniform DirectionalLight sun;

const float PI = 3.14159265359;

float inShadow(vec4 shadowspace) {
    float bias = 0.01;
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
	vec3 N = normal;
	vec3 V = normalize(-fragPos);
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
    float distance = length(-fragPos);
	float shadow0 = mix(1.0, ShadowCalculationPCF(0.0001, fragLightSpace0, shadowTextures[0]), inShadow(fragLightSpace0));
    float shadow1 = mix(1.0, ShadowCalculationPCF(0.0002, fragLightSpace1, shadowTextures[1]), inShadow(fragLightSpace1) * (1-inShadow(fragLightSpace0)));
    float shadow2 = mix(1.0, ShadowCalculationPCF(0.0003, fragLightSpace2, shadowTextures[2]), inShadow(fragLightSpace2) * (1-inShadow(fragLightSpace1)));
    float shadow3 = mix(1.0, ShadowCalculationPCF(0.0004, fragLightSpace3, shadowTextures[3]), inShadow(fragLightSpace3) * (1-inShadow(fragLightSpace2)));
    float shadow = shadow0 * shadow1 * shadow2 * shadow3;
	
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
	}
    vec3 color = Lo;
    return max(color, vec3(0));
}
vec3 postProcess(vec2 texCoord) {
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, texCoord).rgb;
    vec3 Normal = normalize(texture(gNormal, texCoord).rgb);
    vec3 Albedo = texture(gAlbedo, texCoord).rgb;
	vec4 RoughnessMetallic = texture(gRoughnessMetallic, texCoord);
	float Roughness = RoughnessMetallic.r;
	float Metallic = clamp(RoughnessMetallic.g, 0.0, 1.0);
    float emission = RoughnessMetallic.a;
    float ao = RoughnessMetallic.b;
    vec3 light = lighting(FragPos, Albedo, Normal, Roughness, Metallic, 1.0);
    //vec3 light = Albedo;
	if (length(FragPos) > 0.0) {
		return light + (emission * Albedo);
	}
	else {
		discard;
	}
}