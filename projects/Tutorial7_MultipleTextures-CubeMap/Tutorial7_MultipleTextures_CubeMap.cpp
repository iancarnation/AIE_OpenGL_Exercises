#include "Tutorial7_MultipleTextures_CubeMap.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <stb_image.h>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial7_MultipleTextures_CubeMap::Tutorial7_MultipleTextures_CubeMap()
{

}

Tutorial7_MultipleTextures_CubeMap::~Tutorial7_MultipleTextures_CubeMap()
{

}

bool Tutorial7_MultipleTextures_CubeMap::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH/(float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// cube map stuff -----------------------------------------------------

	glEnable(GL_TEXTURE_CUBE_MAP);

	// generate an OpenGL texture and bind it as a GL_TEXTURE_CUBE_MAP
	glGenTextures(1, &m_cubemap_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);

	// set some texture parameters...
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// temporary width and height variables
	int width, height, format;

	// get pixels for each face of the cube and send the data to the graphics card
	unsigned char* pixels_top = stbi_load("../../assets/textures/cubemap/top.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_top);

	unsigned char *pixels_bottom = stbi_load("../../assets/textures/cubemap/bottom.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_bottom);

	unsigned char *pixels_north = stbi_load("../../assets/textures/cubemap/north.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_north);

	unsigned char *pixels_south = stbi_load("../../assets/textures/cubemap/south.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_south);

	unsigned char *pixels_east = stbi_load("../../assets/textures/cubemap/east.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_east);

	unsigned char *pixels_west = stbi_load("../../assets/textures/cubemap/west.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_west);

	// delete the original image data
	free(pixels_top);
	free(pixels_bottom);
	free(pixels_north);
	free(pixels_south);
	free(pixels_east);
	free(pixels_west);

	// Generate Cube
	// vertices for vbo
	float cube_vertices[] = 
	{
	//	x,	  y,	z
		-1.0, 1.0, 1.0,
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0,
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,

	};

	unsigned short cube_indices[] =
	{
		0, 1, 2, 0, 2, 3,
		3, 2, 6, 3, 6, 7,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 4, 1, 0,
		0, 3, 7, 0, 7, 4,
		1, 2, 6, 1, 6, 5,
	};

	// OpenGL: generate buffer and array objects
	glGenBuffers(1, &m_skybox_vbo);
	glGenBuffers(1, &m_skybox_ibo);
	glGenVertexArrays(1, &m_skybox_vao);

	// send the data
	glBindVertexArray(m_skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_skybox_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skybox_ibo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	// setup vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	// load skybox shader
	const char* aszInputs[] = { "Position" };
	const char* aszOutputs[] = { "outColor" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("../../assets/shaders/skybox.vert", GL_VERTEX_SHADER);
	GLuint fshader = Utility::loadShader("../../assets/shaders/skybox.frag", GL_FRAGMENT_SHADER);

	m_skybox_shader = Utility::createProgram(vshader, 0, 0, 0, fshader, 1, aszInputs, 1, aszOutputs);

	// --------------------------------------------------------------------

	return true;
}

void Tutorial7_MultipleTextures_CubeMap::onUpdate(float a_deltaTime) 
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );

	// add a 20x20 grid on the XZ-plane
	for ( int i = 0 ; i < 21 ; ++i )
	{
		Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
		
		Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	}

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial7_MultipleTextures_CubeMap::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// draw the skybox
	glUseProgram(m_skybox_shader);

	glm::mat4 MVP = m_projectionMatrix * viewMatrix * glm::scale(glm::vec3(5, 5, 5));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);

	glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "MVP"), 1, false, glm::value_ptr(MVP));
	glUniform1i(glGetUniformLocation(m_skybox_shader, "CubeMap"), 1);

	glBindVertexArray(m_skybox_vao);
	glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_SHORT, 0);
}

void Tutorial7_MultipleTextures_CubeMap::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial7_MultipleTextures_CubeMap();
	
	if (app->create("AIE - Tutorial7_MultipleTextures-CubeMap",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}