#include "Connection.hpp"
#include "CommandsParser.hpp"
#include "Server.hpp"
#include <sstream>
#include <core_api/Arm.h>
#include <core_api/Disarm.h>
#include <core_api/TakeOff.h>
#include <core_api/Land.h>
#include <core_api/VelocitySet.h>
#include <core_api/PositionSet.h>
#include <core_api/AttitudeSet.h>
#include "Base32.hpp"

namespace srv {

Connection::Connection(asio::ip::tcp::socket s)
	: m_Socket(std::move(s))
	, m_GetBuffer()
	, m_PutBuffer()
	, m_Stream(this)
    , m_ProcessCommandsStrand(Server::instance().ios())
    , m_ProcessCommandsYieldContext(nullptr)
{
	initBuffers();
}

Connection::~Connection()
{
}

void Connection::startProcessingCommands()
{
	asio::spawn(
	  m_ProcessCommandsStrand,
	  std::bind(
		&Connection::processCommands,
		shared_from_this(),
		std::placeholders::_1
	  )
	);
}

void Connection::processCommands(asio::yield_context yctx)
{
	m_ProcessCommandsYieldContext = &yctx;

	CONN_LOG(debug) << "processing commands ...";

	while(true)
	{
		std::string line;
		if(system::error_code rle = readLine(line))
		{
			CONN_LOG(error) << "failed to read a line: " << rle;
			break;
		}

		system::error_code result;
		std::ostringstream data;

		cmd::Command command;
		if(system::error_code pe = cmd::parse(line, command))
		{
			result = pe;
			CONN_LOG(error) << "failed to parse a command: " << pe;
		}
		else
		{
			switch(command.which())
			{
			case 0:
				result = handleArm(boost::get<cmd::Arm>(command), data);
				break;

			case 1:
				result = handleDisarm(boost::get<cmd::Disarm>(command), data);
				break;

			case 2:
				result = handleTakeOff(boost::get<cmd::TakeOff>(command), data);
				break;

			case 3:
				result = handleLand(boost::get<cmd::Land>(command), data);
				break;

			case 4:
				result = handlePositionSetpoint(boost::get<cmd::PositionSetpoint>(command), data);
				break;

			case 5:
				result = handleVelocitySetpoint(boost::get<cmd::VelocitySetpoint>(command), data);
				break;

			case 6:
				result = handleAttitudeSetpoint(boost::get<cmd::AttitudeSetpoint>(command), data);
				break;

			case 7:
				result = handleGetImage(boost::get<cmd::GetImage>(command), data);
				break;

			default:
				CONN_LOG(error) << "unknown command received: " << command.which();
			}
		}

		m_Stream << "result:" << result.value() << " message:\"" << result.message() << "\"\r\n";
		m_Stream << data.str() << "\r\n";
		m_Stream.flush();
	}

	m_ProcessCommandsYieldContext = nullptr;

	CONN_LOG(debug) << "processing commands done!";

}

system::error_code Connection::handleArm(cmd::Arm const &arm, std::ostream &dos)
{
	CONN_LOG(debug) << "received: arm()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::Arm>("/flytsim/navigation/arm");

	core_api::Arm armCall;
	if(!client.call(armCall))
	{
		CONN_LOG(error) << "arm() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleDisarm(cmd::Disarm const &disarm, std::ostream &dos)
{
	CONN_LOG(debug) << "received: disarm()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::Disarm>("/flytsim/navigation/disarm");

	core_api::Disarm disarmCall;
	if(!client.call(disarmCall))
	{
		CONN_LOG(error) << "disarm() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleTakeOff(cmd::TakeOff const &takeOff, std::ostream &dos)
{
	CONN_LOG(debug) << "received: take_off(" << takeOff.altitude << ")";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::TakeOff>("/flytsim/navigation/take_off");

	core_api::TakeOff takeOffCall;
	takeOffCall.request.takeoff_alt = takeOff.altitude;

	if(!client.call(takeOffCall))
	{
		CONN_LOG(error) << "take_off() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleLand(cmd::Land const &land, std::ostream &dos)
{
	CONN_LOG(debug) << "received: land()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::Land>("/flytsim/navigation/land");

	core_api::Land landCall;

	if(land.common.async)
		landCall.request.async = land.common.async.get()?1:0;

	if(!client.call(landCall))
	{
		CONN_LOG(error) << "land() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handlePositionSetpoint(cmd::PositionSetpoint const &positionSetpoint, std::ostream &dos)
{
	CONN_LOG(debug) << "received: position_setpoint()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::VelocitySet>("/flytsim/navigation/position_set");

	core_api::PositionSet positionSetCall;

	positionSetCall.request.twist.twist.linear.x = positionSetpoint.position.x;
	positionSetCall.request.twist.twist.linear.y = positionSetpoint.position.y;
	positionSetCall.request.twist.twist.linear.z = positionSetpoint.position.z;
	if(positionSetpoint.yaw)
		positionSetCall.request.twist.twist.angular.z = positionSetpoint.yaw.get();
	positionSetCall.request.yaw_valid = positionSetpoint.yaw?1:0;
	positionSetCall.request.relative = positionSetpoint.relative?positionSetpoint.relative.get():false;
	positionSetCall.request.body_frame = positionSetpoint.body_frame?positionSetpoint.body_frame.get():false;
	positionSetCall.request.async = 1;

	if(!client.call(positionSetCall))
	{
		CONN_LOG(error) << "position_setpoint() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleVelocitySetpoint(cmd::VelocitySetpoint const &velocitySetpoint, std::ostream &dos)
{
	CONN_LOG(debug) << "received: velocity_setpoint()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::VelocitySet>("/flytsim/navigation/velocity_set");

	core_api::VelocitySet velocitySetCall;

	velocitySetCall.request.twist.twist.linear.x = velocitySetpoint.velocity.x;
	velocitySetCall.request.twist.twist.linear.y = velocitySetpoint.velocity.y;
	velocitySetCall.request.twist.twist.linear.z = velocitySetpoint.velocity.z;
	velocitySetCall.request.twist.twist.angular.x = 0.0f;
	velocitySetCall.request.twist.twist.angular.y = 0.0f;
	if(velocitySetpoint.yaw_rate)
		velocitySetCall.request.twist.twist.angular.z = velocitySetpoint.yaw_rate.get();
	velocitySetCall.request.yaw_rate_valid = velocitySetpoint.yaw_rate?1:0;
	velocitySetCall.request.relative = velocitySetpoint.relative?velocitySetpoint.relative.get():false;
	velocitySetCall.request.body_frame = velocitySetpoint.body_frame?velocitySetpoint.body_frame.get():false;
	velocitySetCall.request.async = 1;

	if(!client.call(velocitySetCall))
	{
		CONN_LOG(error) << "velocity_setpoint() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleAttitudeSetpoint(cmd::AttitudeSetpoint const &attitudeSetpoint, std::ostream &dos)
{
	CONN_LOG(debug) << "received: attitude_setpoint()";
	ros::ServiceClient client = Server::instance().getROSHandle()
		->serviceClient<core_api::AttitudeSet>("/flytsim/navigation/attitude_set");

	core_api::AttitudeSet attitudeSetCall;

	attitudeSetCall.request.pose.twist.angular.x = attitudeSetpoint.rpy.x;
	attitudeSetCall.request.pose.twist.angular.y = attitudeSetpoint.rpy.y;
	attitudeSetCall.request.pose.twist.angular.z = attitudeSetpoint.rpy.z;
	attitudeSetCall.request.thrust = attitudeSetpoint.thrust;

	if(!client.call(attitudeSetCall))
	{
		CONN_LOG(error) << "attitude_setpoint() failed!";
		return make_error_code(system::errc::io_error);
	}

	return system::error_code();
}

system::error_code Connection::handleGetImage(cmd::GetImage const &getImage, std::ostream &dos)
{
	CONN_LOG(debug) << "received: get_image()";
	sensor_msgs::ImageConstPtr img = Server::instance().getROSImage();
	if(!img)
	{
		CONN_LOG(error) << "get_image() failed!";
		return make_error_code(system::errc::no_stream_resources);
	}

	dos <<
		"width:" << img->width << " height:" << img->height << " size:" << img->data.size() << " data:";
	base32::encode(img->data.data(), img->data.size(), dos);

	return system::error_code();
}

system::error_code Connection::readLine(std::string &line)
{
	static char const delim[2] = {'\r', '\n'};

	std::size_t match = 0;
	char c;
	while(m_Stream.get(c).good())
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

void Connection::initBuffers()
{
	setg(&m_GetBuffer[0], &m_GetBuffer[0] + PUTBACK_MAX, &m_GetBuffer[0] + PUTBACK_MAX);
	setp(&m_PutBuffer[0], &m_PutBuffer[0] + m_PutBuffer.size());
}

Connection::int_type Connection::underflow()
{
	BOOST_ASSERT(m_ProcessCommandsYieldContext);

	if(gptr() == egptr())
	{
	  system::error_code err;
	  std::size_t bytes = m_Socket.async_read_some(
		asio::buffer(asio::buffer(m_GetBuffer) + PUTBACK_MAX),
		(*m_ProcessCommandsYieldContext)[err]
	  );

	  if(err)
	  {
		if(!bytes)
		  return traits_type::eof();
	  }

	  setg(&m_GetBuffer[0], &m_GetBuffer[0] + PUTBACK_MAX, &m_GetBuffer[0] + PUTBACK_MAX + bytes);
	  return traits_type::to_int_type(*gptr());
	}
	else
	{
	  return traits_type::eof();
	}
}

Connection::int_type Connection::overflow(int_type c)
{
	BOOST_ASSERT(m_ProcessCommandsYieldContext);

	asio::const_buffer buffer = asio::buffer(pbase(), pptr() - pbase());

	while(asio::buffer_size(buffer) > 0)
	{

	  system::error_code err;
	  std::size_t bytes = asio::async_write(
		m_Socket,
		asio::buffer(buffer),
		(*m_ProcessCommandsYieldContext)[err]
	  );

	  if(err)
	  {
		if(!bytes)
		  return traits_type::eof();
	  }

	  buffer = buffer + bytes;
	}
	setp(&m_PutBuffer[0], &m_PutBuffer[0] + m_PutBuffer.size());

	if(traits_type::eq_int_type(c, traits_type::eof()))
	  return traits_type::not_eof(c);

	*pptr() = traits_type::to_char_type(c);
	pbump(1);
	return c;
}

int Connection::sync()
{
	return overflow(traits_type::eof());
}

} //namespace srv
