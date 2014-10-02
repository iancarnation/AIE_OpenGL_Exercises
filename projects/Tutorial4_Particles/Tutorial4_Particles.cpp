#include "Tutorial4_Particles.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial4_Particles::Tutorial4_Particles()
{

}

Tutorial4_Particles::~Tutorial4_Particles()
{

}

bool Tutorial4_Particles::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH/(float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	// set the clear color and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// create particle emitter
	m_emitter = new ParticleEmitter();
	m_emitter->initialise(1000, 500,
		0.1f, 0.9f, 1, 5, 0.1f, 1.0f,
		glm::vec4(1, 1, 0, 1), glm::vec4(1, 0, 0, 1));

	// load shaders and link shader program
	m_vertShader = Utility::loadShader("../../bin/shaders/particle.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("../../bin/shaders/particle.frag", GL_FRAGMENT_SHADER);

	// our vertex buffer has 2 properties per-vertex
	const char* inputs[] = { "Position", "Color" };
	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 2, inputs);

	return true;
}

void Tutorial4_Particles::onUpdate(float a_deltaTime) 
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

	// particles
	m_emitter->update(a_deltaTime, m_cameraMatrix);

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial4_Particles::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	m_emitter->draw();
}

void Tutorial4_Particles::onDestroy()
{
	// delete the shaders
	glDeleteProgram(m_programID);
	glDeleteShader(m_vertShader);
	glDeleteShader(m_fragShader);

	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial4_Particles();
	
	if (app->create("AIE - Tutorial4_Particles",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}