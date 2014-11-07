#version 330

in vec3 Position;
in vec4 Normals;
in vec2 TexCoord;

out vec4 normals;
out vec2 texCoord;
out vec4 shadowCoord;

uniform mat4 model;
uniform mat4 MVP;

uniform mat4 lightMVP;

const mat4 SHADOW_BIAS_MATRIX = mat4(
					0.5, 0.0, 0.0, 0.0,
					0.0, 0.5, 0.0, 0.0,
					0.0, 0.0, 0.5, 0.0,
					0.5, 0.5, 0.5, 1.0
					);
void main()
{
	normals = model * Normals;
	texCoord = TexCoord;

	shadowCoord = SHADOW_BIAS_MATRIX * lightMVP * vec4(Position, 1);

	gl_Position = MVP * vec4(Position, 1);
}