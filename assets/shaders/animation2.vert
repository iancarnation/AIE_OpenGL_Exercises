#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 indices;
layout(location = 3) in vec4 weights;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec3 tangent;
layout(location = 6) in vec3 binormal;


out vec2 TexCoord;
out vec3 Normal;
out vec3 LightDir;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat3 NormalMatrix;

uniform vec3 LightPosition;

// give bone array a limit
const int MAX_BONES = 128;
uniform mat4 Bones[MAX_BONES];

void main()
{
	// ------ Bones ---------------------------------------------
	// cast indices to integers so they can index an array
	ivec4 index = ivec4(indices);

	// sample bones and blend up to 4
	vec4 P = Bones[index.x] * position * weights.x;
	P += Bones[index.y] * position * weights.y;
	P += Bones[index.z] * position * weights.z;
	P += Bones[index.w] * position * weights.w;
	// ----------------------------------------------------------

	// ------ Lighting ------------------------------------------
	// calculate the surface normals in eye coordinates
	vec3 eye_normal = NormalMatrix * normal;
	vec3 eye_tangent = NormalMatrix * tangent;
	vec3 eye_binormal = NormalMatrix * binormal;

	// get the vertex position and Light position in eye coordinates
	vec3 eye_position = (View * Model * position).xyz;
	vec3 eye_lightPos = (View * Model * vec4(LightPosition, 1)).xyz;

	// calculate the dir to the light from the vertex position
	vec3 lightDir = normalize(eye_lightPos - eye_position);

	// use dot product with light direction and normals
	// this is used to adjust shading based on the direction
	// of the surface
	LightDir = lightDir;
	LightDir.x = dot(lightDir, eye_tangent);
	LightDir.y = dot(lightDir, eye_binormal);
	LightDir.z = dot(lightDir, eye_normal);

	// pass through the surface normal to the frag shader
	Normal = eye_normal;

	// ----------------------------------------------------------

	TexCoord = texCoord;
	gl_Position = (Projection * View * Model) * position;
}