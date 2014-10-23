#version 330

in vec2 TexCoord;

uniform sampler2D DiffuseTexture;

void main()
{
	gl_FragColor = texture2D(DiffuseTexture, TexCoord);
}
