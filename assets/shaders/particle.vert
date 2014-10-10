#version 150

in vec4 Position;
in vec4 Color;

out vec4 color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	color = Color;
	gl_Position = projection * view * Position;
}