#include "CameraView.hpp"

namespace cli { namespace example {

  CameraView::CameraView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_Guard()
    , m_GLVideoTexture(0)
  {

  }

  CameraView::~CameraView()
  {

  }

  void CameraView::setVideoFrame(std::shared_ptr<Image> frame)
  {
    std::lock_guard<std::mutex> lock(m_Guard);
    m_VideoFrame = frame;
    update();
  }

  void CameraView::initializeGL()
  {
    std::lock_guard<std::mutex> lock(m_Guard);
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    glGenTextures(1, &m_GLVideoTexture);
  }

  void CameraView::paintGL()
  {
    std::lock_guard<std::mutex> lock(m_Guard);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateVideoTexture();
    drawVideoTexture();
  }

  void CameraView::drawVideoTexture()
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, m_GLVideoTexture); // Grab the texture from the delegate
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
  }

  void CameraView::updateVideoTexture()
  {
    if(!m_VideoFrame)
      return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_GLVideoTexture);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
    glTexImage2D(GL_TEXTURE_2D,
      0,
      GL_RGB,
      m_VideoFrame->width,
      m_VideoFrame->height,
      0,
      GL_BGR,
      GL_UNSIGNED_BYTE,
      m_VideoFrame->data.data()
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_VideoFrame.reset();
  }

  void CameraView::resizeGL(int w, int h)
  {
    std::lock_guard<std::mutex> lock(m_Guard);
  }

}
}