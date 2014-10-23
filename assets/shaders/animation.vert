#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 indices;
layout(location = 3) in vec4 weights;

out vec2 TexCoord;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// give bone array a limit
const int MAX_BONES = 128;
uniform mat4 Bones[MAX_BONES];

void main()
{
	// cast indices to integers so they can index an array
	ivec4 index = ivec4(indices);

	// sample bones and blend up to 4
	vec4 P = Bones[index.x] * position * weights.x;
	P += Bones[index.y] * position * weights.y;
	P += Bones[index.z] * position * weights.z;
	P += Bones[index.w] * position * weights.w;

	TexCoord = texCoord;
	gl_Position = (Projection * View * Model) * P;
}