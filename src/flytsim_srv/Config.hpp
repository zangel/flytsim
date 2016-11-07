#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include <memory>

#define BOOST_LOG_DYN_LINK 1
#include <boost/atomic.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/bind.hpp>
#include <boost/variant.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/log/trivial.hpp>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber.h>
#include <sensor_msgs/Image.h>

namespace srv
{
	namespace asio = boost::asio;
	namespace system = boost::system;
	template <typename T> using atomic = boost::atomic<T>;
	template <typename T> using optional = boost::optional<T>;
	using boost::system::errc::make_error_code;
	namespace log = boost::log;
}

#endif //CONFIG_HPP
