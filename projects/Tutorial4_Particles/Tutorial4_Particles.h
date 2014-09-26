#pragma once

#include "Application.h"
#include "ParticleSystem.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial4_Particles : public Application
{
public:

	Tutorial4_Particles();
	virtual ~Tutorial4_Particles();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	ParticleEmitter* m_emitter;
};