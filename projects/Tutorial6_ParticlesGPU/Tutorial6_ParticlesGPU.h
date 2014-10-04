#pragma once

#include "Application.h"
#include <glm/ext.hpp>
#include "GPUParticleSystem.h"

// derived application class that wraps up all globals neatly
class Tutorial6_ParticlesGPU : public Application
{
public:

	Tutorial6_ParticlesGPU();
	virtual ~Tutorial6_ParticlesGPU();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	GPUParticleEmitter* m_emitter;

	unsigned int g_texture;

};