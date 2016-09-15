#ifndef EXAMPLE_DRONE_VIEW_HPP
#define EXAMPLE_DRONE_VIEW_HPP

#include "Config.hpp"
#include "ui_DroneVIew.h"

#include "../Connection.hpp"
#include "../Image.hpp"

namespace cli { namespace example {

  class DroneView
    : public QWidget
    , protected Ui::DroneView
  {
  private:
    enum
    {
      DPAD_FORWARD      = 1 << 0,
      DPAD_BACKWARD     = 1 << 1,
      DPAD_LEFT         = 1 << 2,
      DPAD_RIGHT        = 1 << 3,
      DPAD_YAW_LEFT     = 1 << 4,
      DPAD_YAW_RIGHT    = 1 << 5,
      DPAD_THRUST_UP    = 1 << 6,
      DPAD_THRUST_DOWN  = 1 << 7
    };

    Q_OBJECT
  protected:

  public:
    DroneView(QMainWindow *mw);
    ~DroneView();

  protected:
    void      keyPressEvent(QKeyEvent *event) override;
    void	    keyReleaseEvent(QKeyEvent *event) override;

  private slots:
    void onConnectDisconnect();
    void updateUI();

    void onArm();
    void onDisarm();
    void onTakeoff();
    void onLand();

    void onImageTimer();

  private:
    void sendVelocitySetpoint();
    void onImageReceived(std::shared_ptr<Image> img);

  private:
    QTimer *m_ImageTimer;
    std::shared_ptr<Connection> m_Connection;
    uint32_t m_newDPAD;
    uint32_t m_prevDPAD;
    
  };
  
} //namespace example
} //namespace cli

#endif //EXAMPLE_DRONE_VIEW_HPP