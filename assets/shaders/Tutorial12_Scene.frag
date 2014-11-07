#version 330 core

out vec4 FragColor;

in vec4 normals;
in vec2 texCoord;
in vec4 shadowCoord;

uniform vec3 ambientLight;
uniform vec4 lightDir;

uniform sampler2D diffuseMap;
uniform sampler2DShadow shadowMap;

float SHADOW_BIAS = 0.002f;

void main()
{
	float shadowFactor = 1;

	// Convert from screen space to clip space
	vec4 convCoords = shadowCoord / shadowCoord.w;
	convCoords.z += 0.0005;

	// Calculate shadow by testing depth
	
	if ( texture( shadowMap, vec3(convCoords)) < convCoords.z - SHADOW_BIAS )
	{
		shadowFactor = 0;
	}

	// sample diffuse texture and perform lambert lighting
	float NdL = max( 0, dot( normalize(normals), normalize(-lightDir) ));
	vec3 texColor = texture( diffuseMap, texCoord ).rgb;

	// combine diffuse + ambient
	FragColor.rgb = texColor * NdL * shadowFactor + texColor * ambientLight;
	FragColor.a = 1;
}