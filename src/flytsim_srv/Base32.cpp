#include "Base32.hpp"

namespace srv { namespace base32 {

bool encode(void const *data, std::size_t size, std::ostream &os)
{
	uint8_t const *bytes = reinterpret_cast<uint8_t const *>(data);

	if(size > 0)
	{
		int buffer = bytes[0];
		std::size_t next = 1;
		std::size_t bitsLeft = 8;
		while(bitsLeft > 0 || next < size)
		{
			if(bitsLeft < 5)
			{
				if(next < size)
				{
					buffer <<= 8;
					buffer |= bytes[next++] & 0xFF;
					bitsLeft += 8;
				}
				else
				{
					int pad = 5 - bitsLeft;
					buffer <<= pad;
					bitsLeft += pad;
				}
			}

			int index = 0x1F & (buffer >> (bitsLeft - 5));
			bitsLeft -= 5;
			os << "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"[index];
		}
	}
	return true;
}

bool decode(void *data, std::size_t size, std::istream &is)
{
	uint8_t *bytes = reinterpret_cast<uint8_t *>(data);
	int buffer = 0;
	int bitsLeft = 0;
	std::size_t count = 0;
	while(count < size && is.good())
	{
		uint8_t ch = is.get();
		if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-')
		{
			continue;
		}
		buffer <<= 5;

		// Look up one base32 digit
		if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
		{
			ch = (ch & 0x1F) - 1;
		}
		else
		if(ch >= '2' && ch <= '7')
		{
			ch -= '2' - 26;
		}
		else
		{
			is.putback(ch);
			return false;
		}

		buffer |= ch;
		bitsLeft += 5;
		if(bitsLeft >= 8)
		{
			bytes[count++] = buffer >> (bitsLeft - 8);
			bitsLeft -= 8;
		}
	}
	if(count < size)
	{
		bytes[count++] = '\000';
	}
	return count == size;
}

} //namespace base32
} //namespace srv
