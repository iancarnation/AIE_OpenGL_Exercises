#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include "FBXFile.h"

// derived application class that wraps up all globals neatly
class Tutorial9_Animation : public Application
{
public:

	Tutorial9_Animation();
	virtual ~Tutorial9_Animation();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat3 m_NormalMatrix;

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

	unsigned int m_programID, m_vertShader, m_fragShader;

	glm::vec3 m_lightPosition;

	glm::vec3 m_ambientLightColor;
	glm::vec3 m_lightColor;

	

};