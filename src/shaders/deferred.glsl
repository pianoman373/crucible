#include <lighting>

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughnessMetallic;

vec3 postProcess(vec2 texCoord) {
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, texCoord).rgb;
    vec3 Albedo = texture(gAlbedo, texCoord).rgb;
	vec4 RoughnessMetallic = texture(gRoughnessMetallic, texCoord);
    float emission = RoughnessMetallic.a;
    //vec3 light = Albedo;
	if (length(FragPos) > 0.0) {
		return emission * Albedo;
	}
	else {
		discard;
    }
}