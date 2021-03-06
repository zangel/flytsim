#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include <memory>
#include <thread>
#include <mutex>
#include <deque>


#include <boost/atomic.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
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
#include <boost/asio/system_timer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/variant.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>


namespace cli
{
	namespace asio = boost::asio;
	namespace system = boost::system;
	template <typename T> using atomic = boost::atomic<T>;
	template <typename T> using optional = boost::optional<T>;
	using boost::system::errc::make_error_code;
}

#endif //CONFIG_HPP
