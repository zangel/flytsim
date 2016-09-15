#ifndef BASE32_HPP
#define BASE32_HPP

#include "Config.hpp"

namespace cli { namespace base32 {

	extern bool encode(void const *data, std::size_t size, std::ostream &os);
	extern bool decode(void *data, std::size_t size, std::istream &is);

} //namespace base32
} //namespace cli

#endif //BASE32_HPP
