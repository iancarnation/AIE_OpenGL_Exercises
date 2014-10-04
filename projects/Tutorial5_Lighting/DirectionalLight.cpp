#include "DirectionalLight.h"

DirectionalLight::DirectionalLight()
{
	m_v3Facing = glm::vec3(0, -1, 0);
	m_v3Color = glm::vec3(1, 1, 1);
	m_v3SpecularColor = glm::vec3(1, 1, 1);
}

DirectionalLight::DirectionalLight(glm::vec3 a_v3Facing, glm::vec3 a_v3Color)
{
	m_v3Facing = a_v3Facing;
	m_v3Color = a_v3Color;
	m_v3SpecularColor = a_v3Color;
}

DirectionalLight::~DirectionalLight()
{

}

