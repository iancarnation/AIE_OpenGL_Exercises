// #version 420

// layout(location = 0) in vec3 vertexPosition_modelspace;

// uniform mat4 depthMVP;

// void main()
// {
// 	gl_Position = depthMVP * vec4(vertexPosition_modelspace, 1);
// }

#version 150

in vec4 Position;

uniform mat4 depthMVP;

void main()
{
	gl_Position = depthMVP * Position;
}
