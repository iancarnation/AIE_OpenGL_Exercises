#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 LightDir;

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D SpecularTexture;

uniform vec3 AmbientLightColor;
uniform vec3 LightColor;

out vec4 outColor;

void main()
{
	vec4 finalColor = vec4(0,0,0,1);

	// get the color from our textures
	vec3 normalColor = texture(NormalTexture, TexCoord).xyz;
	vec4 diffuseColor = texture(DiffuseTexture, TexCoord);
	vec4 specularColor = texture(SpecularTexture, TexCoord);

	// calculate lighting with dot product
	// N = vNormal of our surface, added to the normal from our NormalMap
	// L = the direction of our light calculated in the vertex shader
	vec3 N = normalize((2.0 * normalColor - 1.0) + Normal);
	vec3 L = LightDir;

	// how bright should this pixel be based on our light and view direction
	float diffuseIntensity = max(0.0, dot(N, L));

	// for simplicity, will use the diffuse texture as the ambient color
	// or this could be a solid color passed as a uniform variable
	vec4 ambientColor = diffuseColor * vec4(AmbientLightColor, 1.0);

	// Add the diffuse contribution blended with the standard texture lookup and add in the 
	// ambient light on top
	//finalColor.rgb = (diffuseIntensity * LightColor.rgb) * diffuseColor.rgb + ambientColor.rgb;
	//finalColor.rgb = (diffuseIntensity * diffuseColor.rgb) + ambientColor.rgb;
	finalColor = (diffuseIntensity * diffuseColor) + ambientColor;

	// specular highlight
	// vec3 reflection = normalize(reflect(-normalize(LightDir), N));
	// float spec = max(0.0, dot(reflection, N));
	// float fSpec = pow(spec, 5.0);

	// // apply the specular
	// finalColor.rgb += vec3(fSpec, fSpec, fSpec) * specularColor.xyz;

	outColor = finalColor;
}
