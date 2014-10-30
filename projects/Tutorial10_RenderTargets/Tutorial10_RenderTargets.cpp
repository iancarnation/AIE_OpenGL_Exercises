#include "Tutorial10_RenderTargets.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
//#include <stb_image.h>
#include <math.h>

using std::sin;
using std::cos;

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial10_RenderTargets::Tutorial10_RenderTargets()
{

}

Tutorial10_RenderTargets::~Tutorial10_RenderTargets()
{

}

bool Tutorial10_RenderTargets::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH/(float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	// set uniform matrices to 0
	m_MVP = glm::mat4(0);
	m_MV = glm::mat4(0);
	m_NormalMatrix = glm::mat3(0);

	// set light position
	m_lightPosition = glm::vec3(0, 1, 0);

	// set light colors
	m_ambientLightColor = glm::vec3(0.8, 0.1, 0.3);
	m_lightColor = glm::vec3(1, 1, 1);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// ---------- TEXTURES & FRAMEBUFFER --------------------------------------

	// create a color texture to be attached to the framebuffer ---
	glGenTextures(1, &m_colorTex);
	// bind the texture for editing
	glBindTexture(GL_TEXTURE_2D, m_colorTex);
	// create the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, 0, GL_RGBA, GL_FLOAT, 0);

	// set the filtering if we intend to sample within a shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// create depth texture ----
	glGenTextures(1, &m_depthTex);
	glBindTexture(GL_TEXTURE_2D, m_depthTex);
	// note the use of 'DEPTH_COMPONENT' arguments
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// create framebuffer object ----
	glGenFramebuffers(1, &m_FBO);
	// bind the framebuffer for editing
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	// attach color ----
	// attach color texture to the 0th color attachment of the framebuffer
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTex, 0);
	// attach depth texture
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTex, 0);

	// tell the framebuffer which color attachments we will be drawing to and how many
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	// if status is not 'complete', there has been an error
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer Error!! Nyaahhgg!!\n");

	// when framebuffer unbound, future render calls are sent to back buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	// -------------------------------------------------------------

	// ----- SHADERS ------------------------------------------------

	// load the shader
	const char* aszInputs[] = { "Position", "Normal", "Tangent", "BiNormal", "TexCoord1" };
	const char* aszOutputs[] = { "outColor" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("../../assets/shaders/Tutorial10_RenderTargets.vert", GL_VERTEX_SHADER);
	GLuint fshader = Utility::loadShader("../../assets/shaders/Tutorial10_RenderTargets.frag", GL_FRAGMENT_SHADER);

	m_programID = Utility::createProgram(vshader, 0, 0, 0, fshader, 5, aszInputs, 1, aszOutputs);

	// free our shader once we built our program
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	// -------------------------------------------------------------

	// ----- FBX ---------------------------------------------------
	m_fbx = new FBXFile();
	m_fbx->load("../../assets/models/soulspear/soulspear.fbx", FBXFile::UNITS_CENTIMETER);
	m_fbx->initialiseOpenGLTextures();
	InitFBXSceneResource(m_fbx);
	// -------------------------------------------------------------

	InitQuad(10);

	m_ReflectionMatrix = glm::reflect3D(m_ReflectionMatrix, m_quadNormal);

	return true;
}

void Tutorial10_RenderTargets::onUpdate(float a_deltaTime) 
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

	UpdateFBXSceneResource(m_fbx);

	m_lightPosition.z = sin(glfwGetTime()) * 2.5f;
	m_lightPosition.x = cos(glfwGetTime()) * 2.5f;

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial10_RenderTargets::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection ** ?? **
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));

	// draw to framebuffer -----------

	//// bind the framebuffer for rendering
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderFBXSceneResource(m_fbx, viewMatrix, m_projectionMatrix);

	//// switch back to back buffer ---
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//DrawQuad();

}

void Tutorial10_RenderTargets::onDestroy()
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
	Application* app = new Tutorial10_RenderTargets();
	
	if (app->create("AIE - Tutorial10_RenderTargets",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}
void Tutorial10_RenderTargets::InitFBXSceneResource(FBXFile *a_pScene)
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
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		// tell our shaders where the information within our buffers lie
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::NormalOffset);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TangentOffset);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::BiNormalOffset);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TexCoord1Offset);

		glBindVertexArray(0);
	}
}
void Tutorial10_RenderTargets::DestroyFBXSceneResource(FBXFile *a_pScene)
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

void Tutorial10_RenderTargets::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial10_RenderTargets::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// enable transparent blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// activate a shader
	glUseProgram(m_programID);

	// get the location of uniforms on the shader
	// vert ---
	GLint uMVP = glGetUniformLocation(m_programID, "MVP");
	GLint uMV = glGetUniformLocation(m_programID, "MV");
	GLint uNormalMatrix = glGetUniformLocation(m_programID, "NormalMatrix");

	GLint uWorld = glGetUniformLocation(m_programID, "World");

	GLint uLightPosition = glGetUniformLocation(m_programID, "LightPosition");

	// frag ---
	GLint uDiffuseTexture = glGetUniformLocation(m_programID, "DiffuseTexture");
	GLint uNormalTexture = glGetUniformLocation(m_programID, "NormalTexture");
	GLint uSpecularTexture = glGetUniformLocation(m_programID, "SpecularTexture");

	GLint uAmbientLightColor = glGetUniformLocation(m_programID, "AmbientLightColor");
	GLint uLightColor = glGetUniformLocation(m_programID, "LightColor");

	// for each mesh in the model
	for (int i = 0; i < a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)mesh->m_userData;

		// calculate matrices ** right spot for this?? **
		m_MVP = a_projection * a_view * (mesh->m_globalTransform);
		m_MV = a_view * (mesh->m_globalTransform);
		m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_MV)));

		glm::mat4 newWorldMatrix = m_ReflectionMatrix * m_World;

		// send matrices to shader
		glUniformMatrix4fv(uMVP, 1, false, glm::value_ptr(m_MVP));
		glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(m_MV));

		glUniformMatrix3fv(uNormalMatrix, 1, false, glm::value_ptr(m_NormalMatrix));

		glUniformMatrix4fv(uWorld, 1, GL_FALSE, (float*)&newWorldMatrix);

		// send light position to shader
		glUniform3fv(uLightPosition, 1, glm::value_ptr(m_lightPosition));

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

void Tutorial10_RenderTargets::InitQuad(int a_size)
{
	// create a simple plane to render
	Utility::build3DPlane(a_size, m_quadVAO, m_quadVBO, m_quadIBO);
	SetQuadNormal(a_size);

	// load shader internally calls glCreateShader...
	m_vertShader = Utility::loadShader("../../assets/shaders/Tutorial3_Texture.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("../../assets/shaders/Tutorial3_Texture.frag", GL_FRAGMENT_SHADER);

	const char* inputs[] = { "position", "colour", "textureCoordinate" };
	m_quadProgramID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 3, inputs);

	// free our shader once we built our program
	glDeleteShader(m_vertShader);
	glDeleteShader(m_fragShader);
}

void Tutorial10_RenderTargets::DrawQuad(glm::mat4 a_viewMatrix, glm::mat4 a_projectionMatrix)
{
	// bind shader to the GPU
	glUseProgram(m_quadProgramID);

	// fetch locations of the view and projection matrices and bind them
	unsigned int location = glGetUniformLocation(m_quadProgramID, "view");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(a_viewMatrix));

	location = glGetUniformLocation(m_quadProgramID, "projection");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(a_projectionMatrix));

	// activate texture slot 0 and bind our texture to it
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_colorTex);

	// fetch the location of the texture sampler and bind it to 0
	location = glGetUniformLocation(m_quadProgramID, "textureMap");
	glUniform1i(location, 0);

	// bind out 3D plane and draw it
	glBindVertexArray(m_quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

}

// sets value to represent normal of reflection plane. **Hardcoding right now..maybe do transform feedback later for dyamism?**
void Tutorial10_RenderTargets::SetQuadNormal(int a_size)
{
	float halfSize = a_size * 0.5f;

	// set vertex values
	glm::vec3 v0(-halfSize, 0, -halfSize);
	glm::vec3 v1(halfSize, 0, -halfSize);
	glm::vec3 v2(halfSize, 0, halfSize);
	glm::vec3 v3(-halfSize, 0, halfSize);

	/* v2        v3
	   +---------+
	   |         |
	   |         |
	   +---------+
	   v0        v1	*/

	// calculate vectors along the two edges that v0 is on
	glm::vec3 edge0(glm::normalize(v1 - v0));
	glm::vec3 edge1(glm::normalize(v2 - v0));

	// normal at v0 is cross product of the edge vectors
	m_quadNormal = glm::cross(edge0, edge1);

}