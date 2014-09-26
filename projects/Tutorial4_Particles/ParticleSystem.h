#pragma once

#include <glm/glm.hpp>
#include <list>

struct Particle 
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;
	float size;
	float lifetime;
	float lifespan;
};

struct ParticleVertex
{
	glm::vec4 position;
	glm::vec4 color;
};

class ParticleEmitter
{
public:

	ParticleEmitter();
	virtual ~ParticleEmitter();

	void ParticleEmitter::initialise(unsigned int a_maxParticles, unsigned int a_emitRate,
		float a_lifetimeMin, float a_lifetimeMax,
		float a_velocityMin, float a_velocityMax,
		float a_startSize, float a_endSize,
		const glm::vec4& a_startColor, const glm::vec4& a_endColor);

	void ParticleEmitter::emit();

	void ParticleEmitter::update(float a_deltaTime, const glm::mat4& a_cameraTransform);

	void ParticleEmitter::draw();

protected:

	std::list<Particle*> m_aliveParticles;
	std::list<Particle*> m_deadParticles;
	Particle*			 m_particles;

	unsigned int m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;
	unsigned int* m_indexData;

	glm::vec3 m_position;

	float m_emitTimer, m_emitRate;

	float m_lifespanMin, m_lifespanMax;

	float m_velocityMin, m_velocityMax;

	float m_startSize, m_endSize;

	glm::vec4 m_startColor, m_endColor;

};