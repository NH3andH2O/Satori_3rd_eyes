#include "verify.hpp"

#include <algorithm>

namespace verify
{
	bool isValidSoftApPassword(const std::string_view password) noexcept
	{
		if (password.empty())
		{
			return true;
		}

		if (password.size() < 8 || password.size() > 63)
		{
			return false;
		}

		return std::ranges::all_of(password, [](const unsigned char character) { return character >= 0x20 && character <= 0x7E; });
	}
} // namespace verify