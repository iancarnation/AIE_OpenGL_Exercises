#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include "FBXFile.h"

// derived application class that wraps up all globals neatly
class Tutorial12_ShadowMap : public Application
{
public:

	Tutorial12_ShadowMap();
	virtual ~Tutorial12_ShadowMap();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	// fbx ------------------------------------
	FBXFile *m_fbx;

	void InitFBXSceneResource(FBXFile *a_pScene);
	void UpdateFBXSceneResource(FBXFile *a_pScene);
	void RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection);
	void DestroyFBXSceneResource(FBXFile *a_pScene);

	struct OGL_FBX_RenderData
	{
		unsigned int VBO;
		unsigned int IBO;
		unsigned int VAO;
	};

	// debug quad ------------------------------
	void InitQuad();
	void DisplayDebugShadow();

	unsigned int m_quadVBO, m_quadVAO;
	unsigned int m_quadProgram, m_quadVert, m_quadFrag;



	unsigned int m_programID;


	void createShadowBuffer();
	void setupLightAndShadowMatrix();
	void updateLightAndShadowMatrix(glm::vec3 a_lightPos, glm::vec4 a_lightDir);

	glm::vec3 m_lightPosition;
	glm::vec4 m_lightDirection;
	glm::mat4 m_shadowViewProjectionMatrix;

	int m_shadowWidth, m_shadowHeight;

	unsigned int m_shadowFramebuffer, m_shadowTexture;

	unsigned int m_shadowProgram, m_shadowVert, m_shadowFrag;

	float scrubber = 0.0f;
	float distCam = 2.5f;

};