#pragma once

#include <string_view>

namespace verify
{
	[[nodiscard]] bool isValidSoftApPassword(std::string_view password) noexcept;
} // namespace verify