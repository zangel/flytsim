#include "Server.hpp"
#include "Connection.hpp"

namespace srv {

Server::Server()
	: m_IOS()
	, m_Work(m_IOS)
	, m_ListenEndpoint(asio::ip::tcp::v4(), DEFAULT_LISTEN_PORT)
	, m_Acceptor(m_IOS)
	, m_AcceptSocket(m_IOS)
	, m_bRunning(false)
	, m_ROSMasterUri(DEFAULT_ROS_MASTER_URI)
	, m_ROSHandle()
	, m_ROSSpinner()
	, m_ROSImageTransport()
	, m_ROSImageTransortSubscriber()
	, m_ROSImage()
{
}

Server::~Server()
{
}

Server& Server::instance()
{
	static Server _instance;
	return _instance;
}

std::uint16_t Server::getPort() const
{
	return m_ListenEndpoint.port();
}

system::error_code Server::setPort(std::uint16_t port)
{
	if(m_bRunning)
		return make_error_code(system::errc::already_connected);

	m_ListenEndpoint.port(port);
	return system::error_code();
}

std::string Server::getROSMasterUri() const
{
	return m_ROSMasterUri;
}

system::error_code Server::setROSMasterUri(std::string uri)
{
	if(m_bRunning)
		return make_error_code(system::errc::already_connected);

	m_ROSMasterUri = uri;
	return system::error_code();
}

std::shared_ptr<ros::NodeHandle> Server::getROSHandle() const
{
	return m_ROSHandle;
}

sensor_msgs::ImageConstPtr Server::getROSImage() const
{
	return m_ROSImage;
}

system::error_code Server::run()
{
	if(m_bRunning)
	{
		SERVER_LOG(error) << "already running!";
		return make_error_code(system::errc::device_or_resource_busy);
	}

	m_bRunning = true;

	if(system::error_code re = startROS())
	{
		SERVER_LOG(error) << "failed to start ROS!";
		m_bRunning = false;
		return re;
	}

	if(system::error_code ae = startAcceptor())
	{
		SERVER_LOG(error) << "failed to start network!";
		stopROS();
		m_bRunning = false;
		return ae;
	}

	m_IOS.run();

	stopAcceptor();
	stopROS();
	m_bRunning = false;

	return system::error_code();
}

void Server::stop()
{
	m_IOS.stop();
}

asio::io_service& Server::ios()
{
	return m_IOS;
}

system::error_code Server::startAcceptor()
{
	SERVER_LOG(debug) << "starting to listen on: " << m_ListenEndpoint;
	system::error_code err;
	m_Acceptor.open(m_ListenEndpoint.protocol(), err);
	if(err)
	{
		SERVER_LOG(error) << "failed to open listening socket: " << err;
		return err;
	}

	m_Acceptor.set_option(asio::ip::tcp::socket::reuse_address(true));
	m_Acceptor.bind(m_ListenEndpoint, err);
	if(err)
	{
		SERVER_LOG(error) << "failed to bind listening socket: " << err;
		m_Acceptor.close();
		return err;
	}

	m_Acceptor.listen(asio::socket_base::max_connections, err);
	if(err)
	{
		SERVER_LOG(error) << "failed to listen on listening socket: " << err;
		m_Acceptor.close();
		return err;
	}
	doAccept();
	return system::error_code();
}

void Server::stopAcceptor()
{
	m_Acceptor.close();
}

void Server::doAccept()
{
	m_Acceptor.async_accept(
		m_AcceptSocket,
		[this](system::error_code e)
		{

			if(!m_bRunning)
				return;
			if(e)
			{
				SERVER_LOG(warning) << "failed to accept connection: " << e;
			}
			else
			{
				SERVER_LOG(debug) << "remote peer connected: " << m_AcceptSocket.remote_endpoint().address();
				std::shared_ptr<Connection> conn = std::make_shared<Connection>(std::move(m_AcceptSocket));
				conn->startProcessingCommands();
			}
			doAccept();
		}
	);
}

system::error_code Server::startROS()
{


	if(m_ROSMasterUri.empty())
	{
		SERVER_LOG(error) << "ROS master uri is empty!";
		return make_error_code(system::errc::no_link);
	}

	SERVER_LOG(debug) << "starting ROS with master uri: " << m_ROSMasterUri;

	::setenv("ROS_MASTER_URI", m_ROSMasterUri.c_str(), 1);

	ros::M_string rosArgs;
	try
	{
		ros::init(rosArgs, "FlytSimServer", 0);
	}
	catch(...)
	{
		SERVER_LOG(error) << "ros::init() failed!";
		return make_error_code(system::errc::no_link);
	}

	if(!ros::master::check())
	{
		SERVER_LOG(error) << "ros::master::check() failed!";
		return make_error_code(system::errc::no_link);
	}

	m_ROSHandle = std::make_shared<ros::NodeHandle>();
	m_ROSSpinner = std::make_shared<ros::AsyncSpinner>(0);
	m_ROSSpinner->start();

	m_ROSImageTransport = std::make_shared<image_transport::ImageTransport>(*m_ROSHandle);
	m_ROSImageTransortSubscriber = std::make_shared<image_transport::Subscriber>(
		m_ROSImageTransport->subscribe(
			"/flytsim/iris/camera/camera_1/image_raw",
			1,
			std::bind(
				&Server::onROSImageReceived,
	            this,
	            std::placeholders::_1
			)
		)
	);
	m_ROSImage.reset();
	system::error_code();
}

void Server::stopROS()
{
	m_ROSImageTransortSubscriber->shutdown();
	m_ROSImageTransortSubscriber.reset();
	m_ROSImageTransport.reset();
	m_ROSImage.reset();
	m_ROSSpinner->stop();
	m_ROSSpinner.reset();
	m_ROSHandle.reset();
}

void Server::onROSImageReceived(sensor_msgs::ImageConstPtr const &img)
{
	SERVER_LOG(trace) << "ROS image received!";
	m_ROSImage = img;
}

} //namespace srv
