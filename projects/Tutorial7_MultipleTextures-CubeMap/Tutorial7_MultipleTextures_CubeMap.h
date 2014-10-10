#pragma once

#include "Application.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial7_MultipleTextures_CubeMap : public Application
{
public:

	Tutorial7_MultipleTextures_CubeMap();
	virtual ~Tutorial7_MultipleTextures_CubeMap();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int m_cubemap_texture;

	unsigned int m_skybox_vao, m_skybox_vbo, m_skybox_ibo;

	unsigned int m_skybox_shader;

};