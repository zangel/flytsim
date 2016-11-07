#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Config.hpp"
#include "Commands.hpp"

#define CONN_LOG(level) BOOST_LOG_TRIVIAL(level) << "[CONN] "

namespace srv {

class Connection
	: public std::enable_shared_from_this<Connection>
	, protected std::streambuf
{
public:
	friend class Server;

	Connection(asio::ip::tcp::socket s);
	~Connection();

protected:
	void startProcessingCommands();

private:
	void processCommands(asio::yield_context yctx);
	system::error_code handleArm(cmd::Arm const &arm, std::ostream &dos);
	system::error_code handleDisarm(cmd::Disarm const &disarm, std::ostream &dos);
	system::error_code handleTakeOff(cmd::TakeOff const &takeOff, std::ostream &dos);
	system::error_code handleLand(cmd::Land const &land, std::ostream &dos);
	system::error_code handlePositionSetpoint(cmd::PositionSetpoint const &positionSetpoint, std::ostream &dos);
	system::error_code handleVelocitySetpoint(cmd::VelocitySetpoint const &velocitySetpoint, std::ostream &dos);
	system::error_code handleAttitudeSetpoint(cmd::AttitudeSetpoint const &attitudeSetpoint, std::ostream &dos);
	system::error_code handleGetImage(cmd::GetImage const &getImage, std::ostream &dos);

	system::error_code readLine(std::string &line);

	//std::streambuf implementation
	void initBuffers();
	int_type underflow();
	int_type overflow(int_type c);
	int sync();

private:
	asio::ip::tcp::socket m_Socket;
	enum { PUTBACK_MAX = 8 };
	enum { BUFFER_SIZE = 8192 };

	asio::detail::array<char, BUFFER_SIZE> m_GetBuffer;
	asio::detail::array<char, BUFFER_SIZE> m_PutBuffer;

	std::iostream m_Stream;

	asio::strand m_ProcessCommandsStrand;
	asio::yield_context *m_ProcessCommandsYieldContext;


};

} //namespace srv

#endif //CONNECTION_HPP
