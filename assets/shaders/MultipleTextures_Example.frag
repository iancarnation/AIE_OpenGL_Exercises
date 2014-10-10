#version 330

// values sent from the vertex shader
in vec2 vUV;
in vec4 vColor;

out vec4 outColor;

uniform sampler2D DiffuseTexture;
uniform sampler2D DiffuseTextureChrome;
uniform sampler2D DecayTexture;

uniform float DecayValue;

void main()
{
	vec4 mainDiffuse = texture2D(DiffuseTexture, vUV.xy) * vColor;
	vec4 chromeDiffuse = texture2D(DiffuseTextureChrome, vUV.xy);

	float decay = 0.0;

	if(texture2D(DecayTexture, vUV.xy).r <= DecayValue)
		decay = 1.0;

	//mainDiffuse.a = decay;

	outColor = mix(mainDiffuse, chromeDiffuse, decay);
}