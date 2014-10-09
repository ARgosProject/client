#ifndef MAINGLWINDOW_H
#define MAINGLWINDOW_H

#include <opencv2/opencv.hpp>
#include <glm/mat4x4.hpp>

#include "paper.hpp"

#include "Singleton.h"
#include "EGLWindow.h"
#include "GfxProgram.h"
#include "GraphicComponent.h"
#include "TaskDelegation.h"

/**
 * The OpenGL ES 2.0 context
 * Addresses all rendering stuff and updates the properties of all
 * graphic components
 */
class GLContext : public EGLWindow, public Singleton<GLContext> {
  typedef std::vector<GraphicComponent*> GraphicComponentList; /// An alias for a vector of graphic
                                                               /// components pointers

public:
  /**
   * Constructs a new OpenGL ES 2.0 context
   * @param config The config we want for the context. If NULL, a default
   * config is built
   */
  GLContext(EGLconfig* config = NULL);

  /**
   * Destroys the OpenGL ES 2.0 context
   */
  ~GLContext();

  /**
   * The main drawing function
   */
  void render() override;

  /**
   * Passes the number of frames per second to the context in order to render it
   * @param fps The number of FPS (Frames Per Second) of the application
   */
  void setFps(float fps);

  /**
   * Updates the Model View matrix of all graphic components according to a paper
   * @param paper The paper we want to center all the graphic components
   */
  void update(paper_t& paper);

  /**
   * Passes a retrieved frame from the camera to stream it on video stream trigger
   * @param currentFrame A frame from the camera
   */
  void setFrame(cv::Mat* currentFrame);

  /**
   * Retrieves the reference of the saved frame
   * @return the saved frame
   */
  cv::Mat& getFrame();

  /**
   * Sets the projection matrix used to update the graphic components transforms
   * @param projectionMatrix The projection matrix used by the context to update the graphic
   * components transforms
   */
  void setProjectionMatrix(glm::mat4 projectionMatrix);

  /**
   * Retrieves the reference of the saved projection matrix
   * @return the projection matrix
   */
  const glm::mat4& getProjectionMatrix() const;

  /**
   * Sets up specific properties for the context
   */
  void start() override;

  /**
   * A utility method used to draw an axis
   */
  void makeAxis(GLfloat axis_length);

  /**
   * A utility method used to draw the 4 corners of the paper
   */
  void makeCorners(GLfloat axis_length, GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);

private:
  std::vector<GraphicComponent*> _gc; ///< The list of graphic components to be updated
  cv::Mat* _frame; ///< The current frame of the camera used on video streaming
  glm::mat4 _projectionMatrix; ///< The projection matrix used to update the graphic components transformations
};

#endif