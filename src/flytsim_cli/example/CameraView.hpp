#ifndef EXAMPLE_CAMERA_VIEW_HPP
#define EXAMPLE_CAMERA_VIEW_HPP

#include "Config.hpp"
#include "../Image.hpp"

namespace cli { namespace example {

  class CameraView
    : public QOpenGLWidget
    , protected QOpenGLFunctions
  {
    Q_OBJECT
  public:
    CameraView(QWidget *parent = 0);
    ~CameraView();

    void setVideoFrame(std::shared_ptr<Image> frame);

  protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void drawVideoTexture();
    void updateVideoTexture();



  private:
    mutable std::mutex m_Guard;
    GLuint m_GLVideoTexture;
    std::shared_ptr<Image> m_VideoFrame;
  };


} //namespace example
} //namespace cli

#endif //EXAMPLE_CAMERA_VIEW_HPP