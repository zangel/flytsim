#ifndef COMMANDSPARSER_HPP
#define COMMANDSPARSER_HPP

#include "Config.hpp"
#include "Commands.hpp"

namespace srv { namespace cmd {

extern system::error_code parse(std::string const &str, Command &command);


} //namespace cmd
} //namespace srv

#endif //COMMANDSPARSER_HPP
