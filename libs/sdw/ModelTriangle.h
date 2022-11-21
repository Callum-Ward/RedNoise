#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "TextureMap.h"

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	std::array<TexturePoint, 3> texturePoints{};
	TextureMap textureMap;
	int isTexture;
	Colour colour{};
	glm::vec3 normal{};
	std::array<glm::vec3, 3> normals{};

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour);
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, std::string textureMapName);

	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
