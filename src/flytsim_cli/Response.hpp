#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Config.hpp"

namespace cli { namespace response {

  struct Result
  {
    int result;
    std::string message;
  };
    
  struct Image
  {
    int width;
    int height;
    int size;
    std::string data;
  };

}
}

#endif //RESPONSE_HPP