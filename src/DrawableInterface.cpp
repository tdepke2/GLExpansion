#include "DrawableInterface.h"
#include "Shader.h"

DrawableInterface::~DrawableInterface() {}

void DrawableInterface::applyInstanceBuffer(unsigned int startIndex) const {}

void DrawableInterface::draw(const Shader& shader, const glm::mat4& modelMtx) const {}

void DrawableInterface::drawGeometry() const {}

void DrawableInterface::drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const {}

void DrawableInterface::drawInstanced(const Shader& shader, unsigned int count) const {}

void DrawableInterface::drawGeometryInstanced(const Shader& shader, unsigned int count) const {}
