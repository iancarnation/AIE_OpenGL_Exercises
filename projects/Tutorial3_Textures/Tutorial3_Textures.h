#pragma once

#include "Application.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial3_Textures : public Application
{
public:

	Tutorial3_Textures();
	virtual ~Tutorial3_Textures();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int m_vertShader;
	unsigned int m_fragShader;
	unsigned int m_programID;

	unsigned int m_vbo;
	unsigned int m_vao;
	unsigned int m_ibo;

	unsigned int m_texture;
};