#include "Application.hpp"
#include "../Service.hpp"

int main(int argc, char *argv[])
{
  cli::Service::instance().ios();
  return cli::example::Application(argc, argv).exec();
}