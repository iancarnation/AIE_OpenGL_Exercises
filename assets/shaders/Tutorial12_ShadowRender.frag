// #version 420

// layout(location = 0) out float fragmentdepth;

// void main()
// {
// 	fragmentdepth = gl_FragCoord.z;
// }

#version 150

out float depth;

void main()
{
	depth = gl_FragCoord.z;
}