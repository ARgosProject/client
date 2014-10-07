#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GraphicComponent.h"
#include "MainGLWindow.h"

GraphicComponent::GraphicComponent()
  : _modelViewMatrix(glm::mat4(1.0f)), _projectionMatrix(glm::mat4(1.0f)),
		_vertexHandler(-1), _texHandler(-1), _samplerHandler(-1), _colorHandler(-1),
		_mvpHandler(-1) {

}

GraphicComponent::~GraphicComponent() {

}

void GraphicComponent::loadGLProgram(const std::string& vertexShader, const std::string& fragmentShader) {
  _shader.loadShaders(vertexShader, fragmentShader);
  this->setUpShader();
}

void GraphicComponent::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  // Any way to optimize this?
  _shader.useProgram();
  glUniform4f(_colorHandler, r, g, b, a);
}

void GraphicComponent::rotate(GLfloat angle, glm::vec3 const & axis) {
  _modelViewMatrix *= glm::rotate(glm::mat4(1.0f), angle, axis);
}

void GraphicComponent::scale(glm::vec3 const & factors) {
  _modelViewMatrix *= glm::scale(glm::mat4(1.0f), factors);
}

void GraphicComponent::translate(glm::vec3 const & translation) {
  _modelViewMatrix *= glm::translate(glm::mat4(1.0f), translation);
}

void GraphicComponent::setModelViewMatrix(glm::mat4 modelViewMatrix) {
  _modelViewMatrix = modelViewMatrix;
}

void GraphicComponent::setProjectionMatrix(glm::mat4 projectionMatrix) {
  _projectionMatrix = projectionMatrix;
}