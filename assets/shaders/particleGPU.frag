#version 150

in vec4 Color;
//in vec2 TexCoord;

uniform sampler2D textureMap;

void main()
{
	//gl_FragColor = texture2D(textureMap, TexCoord);
	gl_FragColor = Color;
}