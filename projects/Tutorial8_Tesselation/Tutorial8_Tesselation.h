#pragma once

#include "Application.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial8_Tesselation : public Application
{
public:

	Tutorial8_Tesselation();
	virtual ~Tutorial8_Tesselation();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_global;

	unsigned int m_texture, m_displacement;

	unsigned int m_vao, m_vbo, m_ibo;

	unsigned int m_programID, m_vertShader, m_fragShader;
};