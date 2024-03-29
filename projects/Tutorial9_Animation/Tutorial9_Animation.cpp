#include "Tutorial9_Animation.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial9_Animation::Tutorial9_Animation()
{

}

Tutorial9_Animation::~Tutorial9_Animation()
{

}

bool Tutorial9_Animation::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH/(float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	m_NormalMatrix = glm::mat3(0);

	// set light position
	m_lightPosition = glm::vec3(0, 1, 0);

	// set light colors
	m_ambientLightColor = glm::vec3(1,1,1);
	m_lightColor = glm::vec3(1, 1, 1);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// load shader internally calls glCreateShader...
	m_vertShader = Utility::loadShader("../../assets/shaders/animation.vert", GL_VERTEX_SHADER);
	m_fragShader= Utility::loadShader("../../assets/shaders/animation.frag", GL_FRAGMENT_SHADER);

	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader);

	// free our shader once we built our program
	glDeleteShader(m_vertShader);
	glDeleteShader(m_fragShader);

	m_fbx = new FBXFile();
	m_fbx->load("../../assets/models/characters/Pyro/pyro.fbx", FBXFile::UNITS_METER);
	m_fbx->initialiseOpenGLTextures();
	InitFBXSceneResource(m_fbx);

	return true;
}

void Tutorial9_Animation::onUpdate(float a_deltaTime) 
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


	// get skeleton and animation
	FBXSkeleton* skeleton = m_fbx->getSkeletonByIndex(0);
	FBXAnimation* animation = m_fbx->getAnimationByIndex(0);

	// evaluate the animation to update bones
	skeleton->evaluate(animation, Utility::getTotalTime());

	UpdateFBXSceneResource(m_fbx);

	// update light
	m_lightPosition.z = sin(glfwGetTime()) * 2.5f;
	m_lightPosition.x = cos(glfwGetTime()) * 2.5f;

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial9_Animation::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));

	RenderFBXSceneResource(m_fbx, viewMatrix, m_projectionMatrix);
}

void Tutorial9_Animation::onDestroy()
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
	Application* app = new Tutorial9_Animation();
	
	if (app->create("AIE - Tutorial9_Animation",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial9_Animation::InitFBXSceneResource(FBXFile *a_pScene)
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
		glEnableVertexAttribArray(0); // position
		glEnableVertexAttribArray(1); // texture coordinates
		glEnableVertexAttribArray(2); // indices
		glEnableVertexAttribArray(3); // blend weights
		glEnableVertexAttribArray(4); // normal
		glEnableVertexAttribArray(5); // tangent
		glEnableVertexAttribArray(6); // binormal

		// tell our shaders where the information within our buffers lie
		// eg: attribute 0 is expected to be the vertices position. it should be 4 floats, representing xyzw
		// eg: attribute 1 is expected to be the vertices texture coordinate. it should be 2 floats, representing U and V
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TexCoord1Offset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::IndicesOffset);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::WeightsOffset);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::NormalOffset);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TangentOffset);
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::BiNormalOffset);

		glBindVertexArray(0);
	}
}
void Tutorial9_Animation::DestroyFBXSceneResource(FBXFile *a_pScene)
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

void Tutorial9_Animation::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial9_Animation::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// enable transparent blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// activate a shader
	glUseProgram(m_programID);

	// get the location of uniforms on the shader
	GLint uDiffuseTexture = glGetUniformLocation(m_programID, "DiffuseTexture");
	GLint uNormalTexture = glGetUniformLocation(m_programID, "NormalTexture");
	GLint uSpecularTexture = glGetUniformLocation(m_programID, "SpecularTexture");

	GLint uModel = glGetUniformLocation(m_programID, "Model");
	GLint uView = glGetUniformLocation(m_programID, "View");
	GLint uProjection = glGetUniformLocation(m_programID, "Projection");
	GLint uNormalMatrix = glGetUniformLocation(m_programID, "NormalMatrix");

	GLint uBones = glGetUniformLocation(m_programID, "Bones");

	GLint uLightPosition = glGetUniformLocation(m_programID, "LightPosition");
	GLint uAmbientLightColor = glGetUniformLocation(m_programID, "AmbientLightColor");
	GLint uLightColor = glGetUniformLocation(m_programID, "LightColor");

	FBXSkeleton* skeleton = m_fbx->getSkeletonByIndex(0);
	skeleton->updateBones();

	// for each mesh in the model
	for (int i = 0; i < a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)mesh->m_userData;

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::NormalTexture]->handle);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::SpecularTexture]->handle);

		// TELL THE SHADER WHICH TEXTURE TO USE
		glUniform1i(uDiffuseTexture, 1);
		glUniform1i(uNormalTexture, 2);
		glUniform1i(uSpecularTexture, 3);

		m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(a_view * (mesh->m_globalTransform))));

		// send the Model, View and Projection Matrices to the shader
		glUniformMatrix4fv(uModel, 1, false, glm::value_ptr(mesh->m_globalTransform));
		glUniformMatrix4fv(uView, 1, false, glm::value_ptr(a_view));
		glUniformMatrix4fv(uProjection, 1, false, glm::value_ptr(a_projection));
		glUniformMatrix3fv(uNormalMatrix, 1, false, glm::value_ptr(m_NormalMatrix));

		glUniformMatrix4fv(uBones, skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);

		// send light position to shader
		glUniform3fv(uLightPosition, 1, glm::value_ptr(m_lightPosition));

		// send light colors
		glUniform3fv(uAmbientLightColor, 1, glm::value_ptr(m_ambientLightColor));
		glUniform3fv(uLightColor, 1, glm::value_ptr(m_lightColor));

		// bind vertex array object
		// remember in the initialize function, we bound the VBO and IBO to the VAO
		// so when we bind the VAO, openGL knows what vertices, indices and vertex attributes 
		// to send to the shader
		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	// reset back to the default active texture
	glActiveTexture(GL_TEXTURE0);

	// finished rendering meshes, disable shader
	glUseProgram(0);
}