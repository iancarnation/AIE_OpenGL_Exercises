#ifndef __GPUPARTICLESYSTEM_H_
#define __GPUPARTICLESYSTEM_H_

#include <glm/ext.hpp>
#include <GL/glew.h>
#include "Utilities.h"
#include <stb_image.h>

struct GPUParticle
{
	GPUParticle() : lifetime(1), lifespan(0) {}

	glm::vec3 position, velocity;
	float lifetime, lifespan;
};

class GPUParticleEmitter
{
public:
	GPUParticleEmitter();
	virtual ~GPUParticleEmitter();

	void initialize(unsigned int a_maxParticles,
		float a_lifetimeMin, float a_lifetimeMax,
		float a_velocityMin, float a_velocityMax,
		float a_startSize, float a_endSize,
		const glm::vec4& a_startColor, const glm::vec4& a_endColor, unsigned int a_texture);

	void draw(const glm::mat4& a_cameraTransform, const glm::mat4& a_projection);

	void setPosition(const glm::vec3 a_position) { m_position = a_position; }

	void loadTexture();

protected:
	void createBuffers();
	void createUpdateShader();
	void createDrawShader();

	GPUParticle* m_particles;

	unsigned int m_maxParticles;

	glm::vec3 m_position;

	float m_lifespanMin;
	float m_lifespanMax;

	float m_velocityMin;
	float m_velocityMax;

	float m_startSize;
	float m_endSize;

	glm::vec4 m_startColor;
	glm::vec4 m_endColor;

	unsigned int m_activeBuffer;
	unsigned int m_vao[2];
	unsigned int m_vbo[2];

	unsigned int m_drawShader;
	unsigned int m_updateShader;

	unsigned int m_texture;
};

#endif  