// #version 330

// in vec2 UV;

// out vec4 FragColor;

// uniform sampler2D shadowMap;

// void main()
// {
// 	FragColor = texture2D(shadowMap,UV);
// }

#version 150

in vec2 UV;

out vec4 FragColor;

uniform sampler2D shadowMap;

void main()
{
	FragColor = texture2D(shadowMap,UV);
}