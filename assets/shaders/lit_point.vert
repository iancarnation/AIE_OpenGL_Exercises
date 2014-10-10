uniform mat4 camera;
uniform mat4 model;

in vec3 position;
in vec3 normal;

out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
	// pass some variables to the fragment shader
	fragPosition = position;
	fragNormal = normal;
	
	// apply all matrix transformations to position
	gl_Position = projection * view * position;
}