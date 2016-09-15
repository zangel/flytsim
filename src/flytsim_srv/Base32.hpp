#ifndef BASE32_HPP
#define BASE32_HPP

#include "Config.hpp"

namespace srv { namespace base32 {

	extern bool encode(void const *data, std::size_t size, std::ostream &os);
	extern bool decode(void *data, std::size_t size, std::istream &is);

} //namespace base32
} //namespace srv

#endif //BASE32_HPP
