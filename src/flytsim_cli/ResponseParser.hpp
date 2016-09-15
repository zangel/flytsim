#ifndef RESPONSE_PARSER_HPP
#define RESPONSE_PARSER_HPP

#include "Config.hpp"
#include "Response.hpp"

namespace cli { namespace response {

  extern system::error_code parseResult(std::string const &str, Result &result);
  extern system::error_code parseImage(std::string const &str, Image &image);


} //namespace response
} //namespace srv

#endif //RESPONSE_PARSER_HPP
