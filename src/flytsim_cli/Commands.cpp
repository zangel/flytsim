#include "Commands.hpp"
#include "Base32.hpp"

namespace cli { namespace cmd {

  Command::Command()
    : m_ResponseResult()
  {
  }

  Command::~Command()
  {
  }

  system::error_code Command::readLine(std::istream &is, std::string &line)
  {
    static char const delim[2] = {'\r', '\n'};

    std::size_t match = 0;
    char c;
    while(is.get(c).good())
    {
      match = (delim[match] == c) ? (match + 1) : 0;

      if(match == 2)
      {
        line.pop_back();
        return system::error_code();
      }

      line.push_back(c);
    }
    return make_error_code(system::errc::illegal_byte_sequence);
  }

  system::error_code Command::readResponseResult(std::istream &is)
  {
    std::string line;
    if(system::error_code rle = readLine(is, line))
      return rle;

    if(system::error_code pe = response::parseResult(line, m_ResponseResult))
    {
      return make_error_code(system::errc::invalid_argument);
    }

    return system::error_code();
  }

  system::error_code Command::readResponseData(std::istream &is)
  {
    std::string line;
    return readLine(is, line);
  }

  void Command::handleIOError(system::error_code ioe)
  {

  }

  Arm::Arm()
    : Command()
  {
  }

  system::error_code Arm::writeRequest(std::ostream &os)
  {
    os << "arm\r\n";
    os.flush();
    return system::error_code();
  }

  Disarm::Disarm()
    : Command()
  {
  }

  system::error_code Disarm::writeRequest(std::ostream &os)
  {
    os << "disarm\r\n";
    os.flush();
    return system::error_code();
  }

  TakeOff::TakeOff(float altitude)
    : Command()
    , altitude(altitude)
  {
  }

  system::error_code TakeOff::writeRequest(std::ostream &os)
  {
    os << "take_off altitude:" << altitude << "\r\n";
    os.flush();
    return system::error_code();
  }

  Land::Land(bool async)
    : Command()
    , async(async)
  {
  }

  system::error_code Land::writeRequest(std::ostream &os)
  {
    os << "land async:" << (async ? "true" : "false") << "\r\n";
    os.flush();
    return system::error_code();
  }

  VelocitySetpoint::VelocitySetpoint(float x, float y, float z, optional<float> yaw_rate, bool relative, bool body_frame)
    : Command()
    , x(x)
    , y(y)
    , z(z)
    , yaw_rate(yaw_rate)
    , relative(relative)
    , body_frame(body_frame)
  {
  }

  system::error_code VelocitySetpoint::writeRequest(std::ostream &os)
  {
    os << "velocity_setpoint velocity:{" << x << "," << y << "," << z << "}";
    if(yaw_rate)
    {
      os << " yaw_rate:" << yaw_rate.get();
    }

    os <<
      " relative:" << (relative ? "true" : "false") <<
      " body_frame:" << (body_frame ? "true" : "false");

    os << "\r\n";
    os.flush();
    return system::error_code();
  }

  AttitudeSetpoint::AttitudeSetpoint(float roll, float pitch, float yaw, float thrust)
    : Command()
    , roll(roll)
    , pitch(pitch)
    , yaw(yaw)
    , thrust(thrust)
  {
  }

  system::error_code AttitudeSetpoint::writeRequest(std::ostream &os)
  {
    os << "attitude_setpoint rpy:{" << roll << "," << pitch << "," << yaw << "} thrust:" << thrust << "\r\n";
    os.flush();
    return system::error_code();
  }

  GetImage::GetImage(Callback callback)
    : Command()
    , callback(callback)
  {

  }

  system::error_code GetImage::writeRequest(std::ostream &os)
  {
    os << "get_image\r\n";
    os.flush();
    return system::error_code();
  }

  system::error_code GetImage::readResponseData(std::istream &is)
  {
    std::string line;
    if(system::error_code rle = readLine(is, line))
      return rle;

    response::Image resImg;

    if(system::error_code pe = response::parseImage(line, resImg))
    {
      return make_error_code(system::errc::invalid_argument);
    }

    if(callback)
    {
      std::shared_ptr<Image> img = std::make_shared<Image>();
      img->width = resImg.width;
      img->height = resImg.height;
      img->data.resize(resImg.size);
      std::istringstream iss(resImg.data);
      if(base32::decode(img->data.data(), img->data.size(), iss))
      {
        callback(img);
      }
    }
    return system::error_code();
  }
  

} //namespace cmd
} //namespace cli