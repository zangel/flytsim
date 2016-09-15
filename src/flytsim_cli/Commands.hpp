#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Config.hpp"
#include "ResponseParser.hpp"
#include "Image.hpp"

namespace cli { namespace cmd {

  class Command
  {
  public:
    Command();
    virtual ~Command();

    static system::error_code readLine(std::istream &is, std::string &line);

    virtual system::error_code writeRequest(std::ostream &os) = 0;
    virtual system::error_code readResponseResult(std::istream &is);
    virtual system::error_code readResponseData(std::istream &is);
    virtual void handleIOError(system::error_code ioe);

  protected:
    response::Result m_ResponseResult;
  };

  class Arm
    : public Command
  {
  public:
    Arm();

    system::error_code writeRequest(std::ostream &os);
  };

  class Disarm
    : public Command
  {
  public:
    Disarm();

    system::error_code writeRequest(std::ostream &os);
  };

  class TakeOff
    : public Command
  {
  public:
    TakeOff(float altitude);

    system::error_code writeRequest(std::ostream &os);

    float altitude;
  };

  class Land
    : public Command
  {
  public:
    Land(bool async);

    system::error_code writeRequest(std::ostream &os);

    bool async;
  };

  class VelocitySetpoint
    : public Command
  {
  public:
    VelocitySetpoint(float x, float y, float z, optional<float> yaw_rate = optional<float>(), bool relative = false, bool body_frame = false);

    system::error_code writeRequest(std::ostream &os);

    float x, y, z;
    optional<float> yaw_rate;
    bool relative;
    bool body_frame;
  };

  class AttitudeSetpoint
    : public Command
  {
  public:
    AttitudeSetpoint(float roll, float pitch, float yaw, float thrust);

    system::error_code writeRequest(std::ostream &os);

    float roll, pitch, yaw;
    float thrust;
  };

  class GetImage
    : public Command
  {
  public:
    typedef std::function<void(std::shared_ptr<Image>)> Callback;

    GetImage(Callback callback);

    system::error_code writeRequest(std::ostream &os);
    system::error_code readResponseData(std::istream &is);

    Callback callback;
  };

} //namespace cmd
} //namespace cli

#endif //COMMANDS_HPP