
#include "Server.hpp"
#include <csignal>


void signalHandler(int signal)
{
	srv::Server::instance().stop();
}


int main(int argc, char *argv[])
{
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGABRT, signalHandler);


	return srv::Server::instance().run().value();
}
