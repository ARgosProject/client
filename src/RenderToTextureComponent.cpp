#include <cassert>
#include <iostream>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RenderToTextureComponent.h"
#include "GLContext.h"

namespace argosClient {

  RenderToTextureComponent::RenderToTextureComponent(GLfloat width, GLfloat height)
    : _framebufferObject(-1), _depthRenderbuffer(-1), _texture(-1),
      _texWidth(width), _texHeight(height) {
    /**
     *    0__1
     *    | /|
     *    |/ |
     *    3__2
     */
    // Screen resolution
    _screenWidth = GLContext::getInstance().getWidth();
    _screenHeight = GLContext::getInstance().getHeight();

    _indices = new GLushort[6] { 0, 1, 2, 0, 2, 3 };
    _vertexData = new GLfloat[20] {
    // X           Y           Z     U     V
      -_texWidth,  _texHeight, 0.0f, 0.0f, 0.0f, // Top-left
       _texWidth,  _texHeight, 0.0f, 1.0f, 0.0f, // Top-right
       _texWidth, -_texHeight, 0.0f, 1.0f, 1.0f, // Bottom-right
      -_texWidth, -_texHeight, 0.0f, 0.0f, 1.0f  // Bottom-left
    };

    // Set the shader
    this->loadGLProgram("shaders/fbo.glvs", "shaders/fbo.glfs");

    this->genFrameBufferObject();
  }

  RenderToTextureComponent::~RenderToTextureComponent() {
    glDeleteFramebuffers(1, &_framebufferObject);
    glDeleteTextures(1, &_texture);
    glDeleteRenderbuffers(1, &_depthRenderbuffer);

    delete [] _indices;
    delete [] _vertexData;

    for(auto& gc : _graphicComponents) {
      delete gc;
    }
    _graphicComponents.clear();
  }

  int RenderToTextureComponent::genFrameBufferObject() {
    // Check if GL_MAX_RENDERBUFFER_SIZE is >= texWidth and texHeight:
    // Cannot use framebuffer objects as we need to create a depth buffer
    // as a renderbuffer object:
    // This is to ensure that we can create a depth renderbuffer successfully and use it as
    // the depth attachment in framebuffer.
    GLint maxRenderbufferSize;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
    assert(maxRenderbufferSize >= _texWidth);
    assert(maxRenderbufferSize >= _texHeight);

    // Generate the framebuffer, renderbuffer and texture object names
    glGenFramebuffers(1, &_framebufferObject);
    glGenRenderbuffers(1, &_depthRenderbuffer);
    glGenTextures(1, &_texture);

    // Bind texture and load the texture mip-level 0
    // Texels are RGB565
    // No texels need to be specified as we are going to draw into the texture
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Bind the framebuffer object and specify texture as color attachment
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferObject);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

    // Bind renderbuffer, create a 16-bit depth buffer and specify depth_renderbufer as depth attachment
    // Width and Height of renderbuffer = Width and Height of the texture
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _texWidth, _texHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    // All went ok?
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Sets FBO projection matrix, so 1 unit = 1 pixel
    _projection = glm::ortho(0.0f, _texWidth, 0.0f, _texHeight, 0.0f, 1.0f);

    return 0;
  }

  void RenderToTextureComponent::renderToTexture() {
    // Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferObject);

    // Set viewport to size of texture map and erase previous image
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _texWidth, _texHeight);

    // Render every GraphicComponents to the FBO
    for(GraphicComponent* gc : _graphicComponents) {
      gc->render();
    }

    // Unbind the FBO so rendering will return to the backbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _screenWidth, _screenHeight);
  }

  void RenderToTextureComponent::drawTexture() {
    // Draw the new texture into the framebuffer
    _shader.useProgram();

    glUniformMatrix4fv(_mvpHandler, 1, GL_FALSE, glm::value_ptr(_projectionMatrix * _modelViewMatrix * _model));

    // Load the vertex data
    glVertexAttribPointer(_vertexHandler, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), _vertexData);
    glEnableVertexAttribArray(_vertexHandler);
    glVertexAttribPointer(_texHandler, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &_vertexData[3]);
    glEnableVertexAttribArray(_texHandler);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);

    // Set the sampler texture unit to 0
    glUniform1i(_samplerHandler, 0);

    // Draw it
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, _indices);
  }

  void RenderToTextureComponent::setUpShader() {
    GLint id = _shader.getId();

    _vertexHandler = glGetAttribLocation(id, "a_position");
    _texHandler = glGetAttribLocation(id, "a_texCoord");
    _samplerHandler = glGetUniformLocation(id, "s_texture");
    _mvpHandler = glGetUniformLocation(id, "u_mvp");
  }

  GraphicComponent* RenderToTextureComponent::getGraphicComponent(int index) {
    return _graphicComponents[index];
  }

  void RenderToTextureComponent::addGraphicComponent(GraphicComponent* graphicComponent) {
    graphicComponent->setProjectionMatrix(_projection);
    _graphicComponents.push_back(graphicComponent);
  }

  void RenderToTextureComponent::specificRender() {
    this->renderToTexture();
    this->drawTexture();
  }

}
