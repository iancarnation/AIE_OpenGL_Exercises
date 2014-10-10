#version 330

in vec3 Position;

out vec3 TexCoord1;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(Position, 1.0);
	TexCoord1 = Position;
}