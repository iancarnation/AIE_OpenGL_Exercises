#include "Tutorial12_ShadowMap.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial12_ShadowMap::Tutorial12_ShadowMap()
{

}

Tutorial12_ShadowMap::~Tutorial12_ShadowMap()
{

}

bool Tutorial12_ShadowMap::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// get window dimensions to calculate aspect ratio
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, width/(float)height, 0.1f, 1000.0f);

	// setup light direction and shadow matrix
	m_lightPosition = glm::vec3(5.0f, 5.0f, 0);
	m_lightDirection = glm::normalize(glm::vec4(-m_lightPosition, 0));

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// shadow map ------------------------------------------------------
	m_shadowVert = Utility::loadShader("../../assets/shaders/Tutorial12_ShadowRender.vert", GL_VERTEX_SHADER);
	m_shadowFrag = Utility::loadShader("../../assets/shaders/Tutorial12_ShadowRender.frag", GL_FRAGMENT_SHADER);
	
	const char* inputs[] = { "Position" };
	const char* outputs[] = { "depth" };
	
	m_shadowProgram = Utility::createProgram(m_shadowVert, 0, 0, 0, m_shadowFrag, 1, inputs, 1, outputs);

	glDeleteShader(m_shadowVert);
	glDeleteShader(m_shadowFrag);

	createShadowBuffer();

	// debug display ---------------------------------------------------
	m_quadVert = Utility::loadShader("../../assets/shaders/Tutorial12_DebugQuad.vert", GL_VERTEX_SHADER);
	m_quadVert = Utility::loadShader("../../assets/shaders/Tutorial12_DebugQuad.frag", GL_FRAGMENT_SHADER);
	
	const char* quadInputs[] = { "Position", "TexCoord" };
	const char* quadOutputs[] = { "FragColor" };

	m_quadProgram = Utility::createProgram(m_quadVert, 0, 0, 0, m_quadFrag, 2, quadInputs, 1, quadOutputs);

	glDeleteShader(m_quadVert);
	glDeleteShader(m_quadFrag);

	// make debug display quad
	InitQuad();

	// scene shader -----------------------------------------------------------
	const char* aszInputs[] = { "Position", "Normals", "TexCoord" };
	const char* aszOutputs[] = { "FragColor" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("../../assets/shaders/Tutorial12_Scene.vert", GL_VERTEX_SHADER);
	GLuint fshader = Utility::loadShader("../../assets/shaders/Tutorial12_Scene.frag", GL_FRAGMENT_SHADER);

	m_programID = Utility::createProgram(vshader, 0, 0, 0, fshader, 3, aszInputs, 1, aszOutputs);

	// free our shader once we built our program
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	// load FBX ----------------------------------------------------------------
	m_fbx = new FBXFile();
	m_fbx->load("../../assets/models/ruinedtank/tank.fbx", FBXFile::UNITS_CENTIMETER);
	//m_fbx->load("../../assets/models/soulspear/soulspear.fbx", FBXFile::UNITS_CENTIMETER);
	//m_fbx->load("../../assets/models/sphere.fbx", FBXFile::UNITS_CENTIMETER);
	m_fbx->initialiseOpenGLTextures();

	InitFBXSceneResource(m_fbx);

	return true;
}

void Tutorial12_ShadowMap::onUpdate(float a_deltaTime) 
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	//Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );

	//// add a 20x20 grid on the XZ-plane
	//for ( int i = 0 ; i < 21 ; ++i )
	//{
	//	Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
	//					 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	//	
	//	Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
	//					 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	//}


	// light movement from @terrehbyte
	if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		scrubber += a_deltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS)
	{
		scrubber -= a_deltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_C) == GLFW_PRESS)
	{
		distCam += a_deltaTime * 10;
	}
	if (glfwGetKey(m_window, GLFW_KEY_V) == GLFW_PRESS)
	{
		distCam -= a_deltaTime * 10;
	}
	m_lightPosition.x = std::cos(scrubber) * distCam;
	m_lightPosition.z = std::sin(scrubber) * distCam;
	updateLightAndShadowMatrix(m_lightPosition, m_lightDirection);

	UpdateFBXSceneResource(m_fbx);

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial12_ShadowMap::onDraw() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );

	// get window dimensions for 2D orthographic projection ** ? ? **
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));

	RenderFBXSceneResource(m_fbx, viewMatrix, m_projectionMatrix);
}

void Tutorial12_ShadowMap::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();

	glDeleteShader(m_programID);

	DestroyFBXSceneResource(m_fbx);
	m_fbx->unload();
	delete m_fbx;
	m_fbx = NULL;

}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial12_ShadowMap();
	
	if (app->create("AIE - Tutorial12_ShadowMap",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial12_ShadowMap::InitFBXSceneResource(FBXFile *a_pScene)
{
	// how many meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// loop through each mesh
	for (int i = 0; i < meshCount; ++i)
	{
		// get the current mesh
		FBXMeshNode *pMesh = a_pScene->getMeshByIndex(i);

		// generate our OGL_FBXRenderData for storing the meshes VBO, IBO, and VAO
		// and assign it to the meshes m_userData pointer so it can be retrieved
		// again within the render function
		OGL_FBX_RenderData *ro = new OGL_FBX_RenderData();
		pMesh->m_userData = ro;

		// OPENGL: generate the VBO, IBO, and VAO
		glGenBuffers(1, &ro->VBO);
		glGenBuffers(1, &ro->IBO);
		glGenVertexArrays(1, &ro->VAO);

		// OpenGL: Bind VAO, then bind the VBO and IBO to the VAO
		glBindVertexArray(ro->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, ro->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ro->IBO);

		// send the vertex data to the VBO
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		// enable the attribute locations that will be used on our shaders
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// tell our shaders where the information within our buffers lie
		// eg: attribute 0 is expected to be the vertices position. it should be 4 floats, representing xyzw
		// eg: attribute 1 is expected to be the vertices color. it should be 4 floats, representing rgba
		// eg: attribute 2 is expected to be the vertices texture coordinate. it should be 2 floats, representing U and V
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::NormalOffset);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TexCoord1Offset);
		glBindVertexArray(0);
	}
}
void Tutorial12_ShadowMap::DestroyFBXSceneResource(FBXFile *a_pScene)
{
	// how many meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// remove meshes
	for (unsigned int i = 0; i < meshCount; i++)
	{
		// Get the current mesh and retrieve the render data we assigned to m_userData
		FBXMeshNode* pMesh = a_pScene->getMeshByIndex(i);
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)pMesh->m_userData;

		// delete the buffers and free memory from the graphics card
		glDeleteBuffers(1, &ro->VBO);
		glDeleteBuffers(1, &ro->IBO);
		glDeleteVertexArrays(1, &ro->VAO);

		// this is memory we creaded earlier in the InitBVXSceneResources function
		// make sure to destroy it
		delete ro;
	}

	// loop through each of the materials
	for (int i = 0; i < matCount; i++)
	{
		// get the current material 
		FBXMaterial *pMaterial = a_pScene->getMaterialByIndex(i);
		for (int j = 0; j < FBXMaterial::TextureTypes_Count; j++)
		{
			// delete the texture if it was loaded
			//if (pMaterial->textureIDs[j] != 0)
			//glDeleteTextures(1, &pMaterial->textureIDs[j]);
		}
	}

}

void Tutorial12_ShadowMap::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial12_ShadowMap::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// Shadow Map -------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shadowProgram);
	//glCullFace(GL_FRONT);

	//setupLightAndShadowMatrix();
	updateLightAndShadowMatrix(m_lightPosition, m_lightDirection);
	GLuint uDepthMVP = glGetUniformLocation(m_shadowProgram, "depthMVP");
	glUniformMatrix4fv(uDepthMVP, 1, false, glm::value_ptr(m_shadowViewProjectionMatrix));

	for (int i = 0; i < a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)mesh->m_userData;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_shadowTexture);

		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glCullFace(GL_BACK);

	// Debug Display
	DisplayDebugShadow();

	// Standard Scene ---------------------------------------------------------

	// get the view mat from world-space camera mat
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	// activate a shader
	glUseProgram(m_programID);

	// bind shadow map texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_shadowTexture);

	// get the location of uniforms on the shader
	GLuint uMVP = glGetUniformLocation(m_programID, "MVP");
	GLuint uModel = glGetUniformLocation(m_programID, "model");
	GLuint uLightMVP = glGetUniformLocation(m_programID, "lightMVP");

	GLuint uAmbientLight = glGetUniformLocation(m_programID, "ambientLight");
	GLuint uLightDir = glGetUniformLocation(m_programID, "lightDir");

	GLuint uDiffuseMap = glGetUniformLocation(m_programID, "diffuseMap");
	GLuint uShadowMap = glGetUniformLocation(m_programID, "shadowMap");

	glm::mat4 MVP;


	// for each mesh in the model
	for (int i = 0; i < a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)mesh->m_userData;

		// Bind Textures
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
		
		glUniform1i(uDiffuseMap, 2);
		glUniform1i(uShadowMap, 3);

		MVP = m_projectionMatrix * viewMatrix * mesh->m_globalTransform;

		// send the Model, View and Projection Matrices to the shader
		glUniformMatrix4fv(uMVP, 1, false, glm::value_ptr(MVP));
		glUniformMatrix4fv(uModel, 1, false, glm::value_ptr(mesh->m_globalTransform));

		glUniformMatrix4fv(uLightMVP, 1, false, glm::value_ptr(m_shadowViewProjectionMatrix * mesh->m_globalTransform));

		glUniform3f(uAmbientLight, 0.5f, 0.5f, 0.5f);
		glUniform4fv(uLightDir, 1, glm::value_ptr(m_lightDirection));

		// bind out vertex array object
		// remember in the initialize function, we bound the VBO and IBO to the VAO
		// so when we bind the VAO, openGL knows what vertices, indices and vertex attributes 
		// to send to the shader
		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	// finished rendering meshes, disable shader
	glUseProgram(0);
}

void Tutorial12_ShadowMap::createShadowBuffer()
{
	// resolution of the texture to put our shadow map in
	m_shadowWidth = 1024;
	m_shadowHeight = 1024;

	// framebuffer
	glGenFramebuffers(1, &m_shadowFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);

	// Depth texture
	glGenTextures(1, &m_shadowTexture);
	glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadowTexture, 0);

	glDrawBuffer(GL_NONE); // no color buffer is drawn to

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("depth buffer not created");
	}
	else
	{
		printf("Success! created depth buffer\n");
	}

	// return to back-buffer rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void Tutorial12_ShadowMap::setupLightAndShadowMatrix()
//{
//	// setup light direction and shadow matrix
//	glm::vec3 lightPosition = glm::vec3(5.0f, 5.0f, 0);
//	m_lightDirection = glm::normalize(glm::vec4(-m_lightPosition, 0));
//
//	glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
//	//glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, 0, 20);
//	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-3, 3, -3, 3, -50, 50);
//	m_shadowViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix;
//}

void Tutorial12_ShadowMap::updateLightAndShadowMatrix(glm::vec3 a_lightPos, glm::vec4 a_lightDir)
{
	glm::mat4 depthViewMatrix = glm::lookAt(a_lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	//glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, 0, 20);
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-3, 3, -3, 3, -50, 50);
	m_shadowViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix;
}

void Tutorial12_ShadowMap::InitQuad()
{
	glGenVertexArrays(1, &m_quadVAO);
	glBindVertexArray(m_quadVAO);

	// create a 200x200 2D GUI quad (resize it to screen-space!)
	glm::vec2 size(200, 200);
	size.x /= DEFAULT_SCREENWIDTH;
	size.y /= DEFAULT_SCREENHEIGHT;
	size *= 2;

	// setup the quad in the top corner
	float quadVertices[] = {
		-1.0f, 1.0f - size.y, 0.0f, 1.0f, 0, 0,
		-1.0f + size.x, 1.0f - size.y, 0.0f, 1.0f, 1, 0,
		-1.0f, 1.0f, 0.0f, 1.0f, 0, 1,

		-1.0f, 1.0f, 0.0f, 1.0f, 0, 1,
		-1.0f + size.x, 1.0f - size.y, 0.0f, 1.0f, 1, 0,
		-1.0f + size.x, 1.0f, 0.0f, 1.0f, 1, 1,
	};

	glGenBuffers(1, &m_quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 6 * 6, quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, ((char*)0) + 16);
	glBindVertexArray(0);
}

void Tutorial12_ShadowMap::DisplayDebugShadow()
{
	glUseProgram(m_quadProgram);
	unsigned int texLoc = glGetUniformLocation(m_quadProgram, "shadowMap");
	glUniform1i(texLoc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
}