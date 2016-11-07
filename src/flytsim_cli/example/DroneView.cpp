#include "DroneView.hpp"
#include "../Commands.hpp"

namespace cli { namespace example {


DroneView::DroneView(QMainWindow *mw)
  : QWidget(mw)
  , Ui::DroneView()
  , m_ImageTimer(new QTimer(this))
  , m_Connection(std::make_shared<Connection>())
  , m_newDPAD(0)
  , m_prevDPAD(0)
{

  m_ImageTimer->setInterval(100);
  connect(m_ImageTimer, SIGNAL(timeout()), SLOT(onImageTimer()));


  setupUi(this);

  m_TakeOffAltitudeLineEdit->setValidator(new QDoubleValidator(0.0, std::numeric_limits<float>::max(), 2));
  connect(m_ConnectButton, SIGNAL(clicked()), this, SLOT(onConnectDisconnect()));
  connect(m_ArmButton, SIGNAL(clicked()), this, SLOT(onArm()));
  connect(m_DisarmButton, SIGNAL(clicked()), this, SLOT(onDisarm()));
  connect(m_TakeOffButton, SIGNAL(clicked()), this, SLOT(onTakeoff()));
  connect(m_LandButton, SIGNAL(clicked()), this, SLOT(onLand()));
  updateUI();
}

DroneView::~DroneView()
{

}

void DroneView::keyPressEvent(QKeyEvent *event)
{
  bool handled = false;

  switch(event->key())
  {
  case 87: //w
    m_newDPAD |= DPAD_FORWARD;
    handled = true;
    break;

  case 83: //s
    m_newDPAD |= DPAD_BACKWARD;
    handled = true;
    break;

  case 65: //a
    m_newDPAD |= DPAD_LEFT;
    handled = true;
    break;

  case 68: //a
    m_newDPAD |= DPAD_RIGHT;
    handled = true;
    break;

  case 61: //=
    m_newDPAD |= DPAD_THRUST_UP;
    handled = true;
    break;

  case 45: //-
    m_newDPAD |= DPAD_THRUST_DOWN;
    handled = true;
    break;

  case 81: //q
    m_newDPAD |= DPAD_YAW_LEFT;
    handled = true;
    break;

  case 69: //e
    m_newDPAD |= DPAD_YAW_RIGHT;
    handled = true;
    break;
  }

  if(handled)
  {
    event->accept();
    if(m_Connection->isConnected())
    {
      sendVelocitySetpoint();
    }
  }
  else
  {
    QWidget::keyPressEvent(event);
  }
}

void DroneView::keyReleaseEvent(QKeyEvent *event)
{
  bool handled = false;
  switch(event->key())
  {
  case 87: //w
    m_newDPAD &= ~DPAD_FORWARD;
    handled = true;
    break;

  case 83: //s
    m_newDPAD &= ~DPAD_BACKWARD;
    handled = true;
    break;

  case 65: //a
    m_newDPAD &= ~DPAD_LEFT;
    handled = true;
    break;

  case 68: //a
    m_newDPAD &= ~DPAD_RIGHT;
    handled = true;
    break;

  case 61: //=
    m_newDPAD &= ~DPAD_THRUST_UP;
    handled = true;
    break;

  case 45: //-
    m_newDPAD &= ~DPAD_THRUST_DOWN;
    handled = true;
    break;

  case 81: //q
    m_newDPAD &= ~DPAD_YAW_LEFT;
    handled = true;
    break;

  case 69: //e
    m_newDPAD &= ~DPAD_YAW_RIGHT;
    handled = true;
    break;
  }

  if(handled)
  {
    event->accept();
    if(m_Connection->isConnected())
    {
      sendVelocitySetpoint();
    }
  }
  else
  {
    QWidget::keyReleaseEvent(event);
  }
}

void DroneView::onConnectDisconnect()
{
  if(m_Connection->isConnected())
  {
    m_ImageTimer->stop();
    m_Connection->disconnect();
  }
  else
  {
    if(!m_Connection->connect(m_ServerAddressLineEdit->text().toStdString()))
      m_ImageTimer->start();
  }

  updateUI();
}

void DroneView::updateUI()
{
  m_ConnectButton->setText(m_Connection->isConnected() ? "Disconnect" : "Connect");
  m_ServerAddressLabel->setEnabled(!m_Connection->isConnected());
  m_ArmButton->setEnabled(m_Connection->isConnected());
  m_DisarmButton->setEnabled(m_Connection->isConnected());
  m_TakeOffButton->setEnabled(m_Connection->isConnected());
  m_TakeOffAltitudeLabel->setEnabled(m_Connection->isConnected());
  m_TakeOffAltitudeLineEdit->setEnabled(m_Connection->isConnected());
  m_LandButton->setEnabled(m_Connection->isConnected());
  m_LandAsyncronouslyCheckBox->setEnabled(m_Connection->isConnected());
 }


void DroneView::onArm()
{
  m_Connection->asyncSendCommand(std::make_shared<cmd::Arm>());
}

void DroneView::onDisarm()
{
  m_Connection->asyncSendCommand(std::make_shared<cmd::Disarm>());
}

void DroneView::onTakeoff()
{
  m_Connection->asyncSendCommand(std::make_shared<cmd::TakeOff>(m_TakeOffAltitudeLineEdit->text().toFloat()));
}

void DroneView::onLand()
{
  m_Connection->asyncSendCommand(std::make_shared<cmd::Land>(m_LandAsyncronouslyCheckBox->isChecked()));
}

void DroneView::onImageTimer()
{
  m_Connection->asyncSendCommand(std::make_shared<cmd::GetImage>(
    std::bind(&DroneView::onImageReceived, this, std::placeholders::_1)
  ));
}

void DroneView::sendVelocitySetpoint()
{
  if(m_newDPAD != m_prevDPAD)
  {
    float roll = 0.0f;
    if(m_newDPAD & DPAD_FORWARD)      roll += 0.5f;
    if(m_newDPAD & DPAD_BACKWARD)     roll -= 0.5f;

    float pitch = 0.0f;
    if(m_newDPAD & DPAD_LEFT)         pitch += 0.5f;
    if(m_newDPAD & DPAD_RIGHT)        pitch -= 0.5f;
    
    float yaw = 0.0f;
    if(m_newDPAD & DPAD_YAW_LEFT)     yaw -= 0.1f;
    if(m_newDPAD & DPAD_YAW_RIGHT)    yaw += 0.1f;

    float thrust = 0.5f;
    if(m_newDPAD & DPAD_THRUST_UP)    thrust += 0.5f;
    if(m_newDPAD & DPAD_THRUST_DOWN)  thrust -= 0.5f;
    
    m_Connection->asyncSendCommand(std::make_shared<cmd::AttitudeSetpoint>(roll, pitch, yaw, thrust));
  }

  m_prevDPAD = m_newDPAD;
    
  m_CommandsCounter->setText(QString("%1").arg(m_Connection->pendingCommandsCount()));
}

void DroneView::onImageReceived(std::shared_ptr<Image> img)
{
  m_CameraView->setVideoFrame(img);
}

} //namespace example
} //namespace cli
