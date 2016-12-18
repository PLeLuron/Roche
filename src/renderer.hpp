#pragma once

#include "planet.h"
#include <glm/glm.hpp>

class Renderer
{
public:
	void setGamma(float gamma) { this->gamma = gamma; }
	virtual void init(
		std::vector<PlanetParameters> planetParams, 
		SkyboxParameters skyboxParam) = 0;
	virtual void render(
		int windowWidth,
		int windowHeight,
		glm::dvec3 viewPos, 
		float fovy,
		glm::dvec3 viewCenter,
		glm::vec3 viewUp,
		std::vector<PlanetState> planetStates) = 0;
	
	virtual void destroy() = 0;

protected:
	float gamma;
};