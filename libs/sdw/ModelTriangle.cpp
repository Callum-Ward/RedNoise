#include "ModelTriangle.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour, std::string surfaceType) :
		vertices({{v0, v1, v2}}), texturePoints(), colour(std::move(trigColour)),surfaceType(surfaceType), isTexture(0), normal() {}

ModelTriangle::ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, std::string textureMapName, std::string surfaceType) :
		vertices({{v0, v1, v2}}), texturePoints(), textureMap(TextureMap(textureMapName)),surfaceType(surfaceType),isTexture(1), normal() {}

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")\n";
	os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")\n";
	os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")\n";
	return os;
}
