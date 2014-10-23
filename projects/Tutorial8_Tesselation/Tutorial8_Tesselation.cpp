#include "Tutorial8_Tesselation.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <stb_image.h>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial8_Tesselation::Tutorial8_Tesselation()
{

}

Tutorial8_Tesselation::~Tutorial8_Tesselation()
{

}

bool Tutorial8_Tesselation::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH/(float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	m_displacementScale = 2.0;
	m_tessLevelIn = 64;
	m_tessLevelOut = 64;

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Utility::build3DPlane(10, m_vao, m_vbo, m_ibo);

	int width, height, format;
	unsigned char* data = nullptr;

	data = stbi_load("../../assets/textures/rock_diffuse.tga", &width, &height, &format, STBI_default);
	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	// convert the stbi format to the actual GL code
	switch (format)
	{
	case STBI_grey: format = GL_LUMINANCE; break;
	case STBI_grey_alpha: format = GL_LUMINANCE_ALPHA; break;
	case STBI_rgb: format = GL_RGB; break;
	case STBI_rgb_alpha: format = GL_RGBA; break;
	};

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	data = stbi_load("../../assets/textures/rock_displacement.tga", &width, &height, &format, STBI_default);
	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	// convert the stbi format to the actual GL code
	switch (format)
	{
	case STBI_grey: format = GL_LUMINANCE; break;
	case STBI_grey_alpha: format = GL_LUMINANCE_ALPHA; break;
	case STBI_rgb: format = GL_RGB; break;
	case STBI_rgb_alpha: format = GL_RGBA; break;
	};

	glGenTextures(1, &m_displacement);
	glBindTexture(GL_TEXTURE_2D, m_displacement);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] data;

	m_vertShader = Utility::loadShader("../../assets/shaders/displace.vert", GL_VERTEX_SHADER);
	m_contShader = Utility::loadShader("../../assets/shaders/displace.cont", GL_TESS_CONTROL_SHADER);
	m_evalShader = Utility::loadShader("../../assets/shaders/displace.eval", GL_TESS_EVALUATION_SHADER);
	m_fragShader = Utility::loadShader("../../assets/shaders/displace.frag", GL_FRAGMENT_SHADER);
	
	m_programID = Utility::createProgram(m_vertShader, m_contShader, m_evalShader, 0, m_fragShader);

	glDeleteShader(m_vertShader);
	glDeleteShader(m_contShader);
	glDeleteShader(m_evalShader);
	glDeleteShader(m_fragShader);

	return true;
}

void Tutorial8_Tesselation::onUpdate(float a_deltaTime) 
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );

	// add a 20x20 grid on the XZ-plane
	/*for ( int i = 0 ; i < 21 ; ++i )
	{
		Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
		
		Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	}*/

	//m_displacementScale = sin(glfwGetTime()) * 2.5f;

	// control tesselation amount with keyboard
	float tessScale = 4000.0;

	if (glfwGetKey(m_window, GLFW_KEY_KP_4) == GLFW_PRESS && m_tessLevelIn > 1)
		m_tessLevelIn -= Utility::tickTimer() * tessScale;
	if (glfwGetKey(m_window, GLFW_KEY_KP_6) == GLFW_PRESS && m_tessLevelIn < 64)
		m_tessLevelIn += Utility::tickTimer() * tessScale;

	if (glfwGetKey(m_window, GLFW_KEY_KP_8) == GLFW_PRESS && m_tessLevelOut < 64)
		m_tessLevelOut += Utility::tickTimer() * tessScale;
	if (glfwGetKey(m_window, GLFW_KEY_KP_2) == GLFW_PRESS && m_tessLevelOut > 1)
		m_tessLevelOut -= Utility::tickTimer() * tessScale;

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();


}

void Tutorial8_Tesselation::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// bind shader to the GPU
	glUseProgram(m_programID);

	// fetch locations of the view and projection matrices and bind them
	GLuint uViewProjection = glGetUniformLocation(m_programID, "viewProjection");
	GLuint uTextureMap = glGetUniformLocation(m_programID, "textureMap");
	GLuint uGlobal = glGetUniformLocation(m_programID, "global");
	GLuint uDisplacementMap = glGetUniformLocation(m_programID, "displacementMap");
	GLuint uDisplacementScale = glGetUniformLocation(m_programID, "displacementScale");
	GLuint uTessLevelIn = glGetUniformLocation(m_programID, "tessLevelIn");
	GLuint uTessLevelOut = glGetUniformLocation(m_programID, "tessLevelOut");

	// activate texture slots and bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_displacement);

	// fetch the location of the texture samplers and bind to appropriate texture slot
	glUniform1i(uTextureMap, 0);
	glUniform1i(uDisplacementMap, 1);

	// bind other uniforms
	glUniformMatrix4fv(uViewProjection, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));

	glUniform1f(uDisplacementScale, m_displacementScale);
	glUniform1f(uTessLevelIn, m_tessLevelIn);
	glUniform1f(uTessLevelOut, m_tessLevelOut);

	// bind 3D plane and draw it
	glBindVertexArray(m_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, nullptr);
}

void Tutorial8_Tesselation::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial8_Tesselation();
	
	if (app->create("AIE - Tutorial8_Tesselation",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}