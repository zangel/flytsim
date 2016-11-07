#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"

#define SERVER_LOG(level) BOOST_LOG_TRIVIAL(level) << "[SERVER] "

namespace srv {

class Connection;

class Server
{
protected:
	Server();

public:
	virtual ~Server();

	static std::uint16_t const DEFAULT_LISTEN_PORT = 12321;
	static constexpr char const * DEFAULT_ROS_MASTER_URI = "http://localhost:11311";

	static Server& instance();

	std::uint16_t getPort() const;
	system::error_code setPort(std::uint16_t port);

	std::string getROSMasterUri() const;
	system::error_code setROSMasterUri(std::string uri);

	std::shared_ptr<ros::NodeHandle> getROSHandle() const;
	sensor_msgs::ImageConstPtr getROSImage() const;


	system::error_code run();
	void stop();

	asio::io_service& ios();

protected:
	system::error_code startAcceptor();
	void stopAcceptor();
	void doAccept();

	system::error_code startROS();
	void stopROS();
	void onROSImageReceived(sensor_msgs::ImageConstPtr const &img);

private:
	asio::io_service m_IOS;
	asio::io_service::work m_Work;
	asio::ip::tcp::endpoint m_ListenEndpoint;
	asio::ip::tcp::acceptor m_Acceptor;
	asio::ip::tcp::socket m_AcceptSocket;
	atomic<bool> m_bRunning;

	std::string m_ROSMasterUri;
	std::shared_ptr<ros::NodeHandle> m_ROSHandle;
	std::shared_ptr<ros::AsyncSpinner> m_ROSSpinner;
	std::shared_ptr<image_transport::ImageTransport> m_ROSImageTransport;
	std::shared_ptr<image_transport::Subscriber> m_ROSImageTransortSubscriber;
	sensor_msgs::ImageConstPtr m_ROSImage;
};

} //namespace srv

#endif //SERVER_HPP
