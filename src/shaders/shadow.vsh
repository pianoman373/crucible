#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in ivec4 vBoneIDs;
layout (location = 5) in vec4 vBoneWeights;

uniform mat4 bones[100];

uniform bool doAnimation;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	vec4 viewPos;

    if (doAnimation) {
        vec4 totalLocalPos = vec4(0.0);

        for(int i=0;i<4;i++){
            mat4 jointTransform = bones[vBoneIDs[i]];
            vec4 posePosition = jointTransform * vec4(vPosition, 1.0);
            totalLocalPos += posePosition * (vBoneWeights[i] + 0.000000001);
        }

        viewPos = lightSpaceMatrix * model * totalLocalPos;
    }
    else {
    	viewPos = lightSpaceMatrix * model * vec4(vPosition, 1.0f);
    }

    gl_Position = viewPos;
}