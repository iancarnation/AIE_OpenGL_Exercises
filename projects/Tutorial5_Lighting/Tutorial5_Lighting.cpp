#include "Tutorial5_Lighting.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial5_Lighting::Tutorial5_Lighting()
{

}

Tutorial5_Lighting::~Tutorial5_Lighting()
{

}

bool Tutorial5_Lighting::onCreate(int a_argc, char* a_argv[]) 
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

	// load shaders and link shader program
	m_vertShader = Utility::loadShader("../../bin/shaders/lit.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("../../bin/shaders/lit.frag", GL_FRAGMENT_SHADER);	
	const char* inputs[] = { "Position", "Normal" };
	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 2, inputs);
	
	m_fbx = new FBXFile();
	m_fbx->load("../../bin/models/stanford/Bunny.fbx");

	createOpenGLBuffers(m_fbx);

	// create directional light
	glm::vec3 dLightDir(0, -.5, 0);
	glm::vec3 dLightColor(0.4, 0.8, 0.1);
	m_dLight = DirectionalLight(dLightDir, dLightColor);

	// ambient light
	m_aLight = glm::vec3(0.1, 0.1, 0.1);

	// point light
	glm::vec3 pLightPosition(0, 1, 0);
	glm::vec3 pLightColor(0.4, 0.8, 0.5);
	m_pLight = PointLight(pLightPosition, pLightColor);

	return true;
}

void Tutorial5_Lighting::onUpdate(float a_deltaTime) 
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

	// update fbx
	//m_fbx->getRoot()->updateGlobalTransform();
}

void Tutorial5_Lighting::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);
	
	// bind shader to GPU
	glUseProgram(m_programID);

	// fetch locations of the view and projection matrices and bind them
	unsigned int location = glGetUniformLocation(m_programID, "view");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(viewMatrix));

	location = glGetUniformLocation(m_programID, "projection");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(m_projectionMatrix));

	// Directional Light -------------------------------------------------
	location = glGetUniformLocation(m_programID, "lightAmbient");
	glUniform3fv(location, 1, glm::value_ptr(m_aLight));

	location = glGetUniformLocation(m_programID, "lightDirection");
	glUniform3fv(location, 1, glm::value_ptr(m_dLight.m_v3Facing));

	location = glGetUniformLocation(m_programID, "lightColor");
	glUniform3fv(location, 1, glm::value_ptr(m_dLight.m_v3Color));

	location = glGetUniformLocation(m_programID, "specularColor");
	glUniform3fv(location, 1, glm::value_ptr(m_dLight.m_v3SpecularColor));

	// Point Light --------------------------------------------------------



	location = glGetUniformLocation(m_programID, "cameraPosition");
	glUniform3fv(location, 1, glm::value_ptr(m_cameraMatrix[3]));


	// bind our vertex array object and draw the mesh
	for (unsigned int i = 0; i < m_fbx->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = m_fbx->getMeshByIndex(i);

		unsigned int* glData = (unsigned int*)mesh->m_userData;

		glBindVertexArray(glData[0]);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}
}

void Tutorial5_Lighting::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial5_Lighting();
	
	if (app->create("AIE - Tutorial5_Lighting",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial5_Lighting::createOpenGLBuffers(FBXFile* a_fbx)
{
	// create the GL VAO/VBO/IBO data for meshes
	for (unsigned int i = 0; i < a_fbx->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = a_fbx->getMeshByIndex(i);

		// storage for the opengl data in 3 unsigned int
		unsigned int* glData = new unsigned int[3];

		glGenVertexArrays(1, &glData[0]);
		glBindVertexArray(glData[0]);

		glGenBuffers(1, &glData[1]);
		glGenBuffers(1, &glData[2]);

		glBindBuffer(GL_ARRAY_BUFFER, glData[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData[2]);

		glBufferData(GL_ARRAY_BUFFER, mesh->m_vertices.size() * sizeof(FBXVertex), mesh->m_vertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->m_indices.size() * sizeof(unsigned int), mesh->m_indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh->m_userData = glData;
	}
}

void Tutorial5_Lighting::cleanupOpenGLBuffers(FBXFile* a_fbx)
{
	// bind our vertex array object and draw the mesh
	for (unsigned int i = 0; i < a_fbx->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = a_fbx->getMeshByIndex(i);

		unsigned int* glData = (unsigned int*)mesh->m_userData;

		glDeleteVertexArrays(1, &glData[0]);
		glDeleteBuffers(1, &glData[1]);
		glDeleteBuffers(1, &glData[2]);

		delete[] glData;
	}
}