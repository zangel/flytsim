
#include "Server.hpp"
#include <csignal>

#include <boost/program_options.hpp>
#include <boost/log/expressions.hpp>

namespace po = boost::program_options;


#define MAIN_LOG(level) BOOST_LOG_TRIVIAL(level) << "[MAIN] "


void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
	case SIGTERM:
	case SIGABRT:
	case SIGQUIT:
		break;

	default:
		MAIN_LOG(error) << "caught signal: " << signal;
		break;
	}

	MAIN_LOG(info) << "stopping the server ...";
	srv::Server::instance().stop();
}


int main(int argc, char *argv[])
{
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGABRT, signalHandler);
	std::signal(SIGQUIT, signalHandler);


	boost::log::trivial::severity_level poLogLevel;
	int poListenPort;
	std::string poROSMasterUri;
	try
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			(
				"help",
				"produce help message"
			)
			(
				"loglevel,l",
				po::value<boost::log::trivial::severity_level>(&poLogLevel)->default_value(srv::log::trivial::info),
				"log level: (trace, debug, info, warning, error, fatal)"
			)
	        (
	        	"port,p",
	        	po::value<int>(&poListenPort)->default_value(srv::Server::DEFAULT_LISTEN_PORT),
	        	"listen on a port number"
	        )
	        (
				"ros,r",
				po::value<std::string>(&poROSMasterUri)->default_value(srv::Server::DEFAULT_ROS_MASTER_URI),
				"ROS master uri"
			);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		po::notify(vm);

		if(vm.count("help"))
		{
			std::cout << "Usage: flytsim_srv [options]\n";
			std::cout << desc;
			return 0;
		}

		srv::log::core::get()->set_filter
		(
			boost::log::trivial::severity >= poLogLevel
		);

		srv::Server::instance().setPort(poListenPort);
		srv::Server::instance().setROSMasterUri(poROSMasterUri);



	}
	catch(std::exception& e)
	{
		std::cout << e.what() << "\n";
		return 1;
	}


	MAIN_LOG(info) << "running the server ...";
	srv::system::error_code rr = srv::Server::instance().run();
	MAIN_LOG(info) << "server exited with result: " << rr;
	return rr.value();
}
