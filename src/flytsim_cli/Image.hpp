#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "Config.hpp"

namespace cli {

  class Image
  {
  public:
    Image();
  
    int width;
    int height;
    std::vector<std::uint8_t> data;
  };

} //namespace cli

#endif //IMAGE_HPP