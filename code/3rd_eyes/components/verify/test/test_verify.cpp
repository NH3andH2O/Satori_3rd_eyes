#include "unity.h"

#include <array>
#include <string>
#include <string_view>

#include "verify.hpp"

TEST_CASE("empty SoftAP password keeps open network valid", "[verify]") { TEST_ASSERT_TRUE(verify::isValidSoftApPassword("")); }

TEST_CASE("SoftAP password length boundaries are enforced", "[verify]")
{
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string(7, 'a')));
	TEST_ASSERT_TRUE(verify::isValidSoftApPassword(std::string(8, 'a')));
	TEST_ASSERT_TRUE(verify::isValidSoftApPassword(std::string(63, 'a')));
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string(64, 'a')));
}

TEST_CASE("printable ASCII boundaries and punctuation are accepted", "[verify]")
{
	TEST_ASSERT_TRUE(verify::isValidSoftApPassword(" !#$%&'~"));
	TEST_ASSERT_TRUE(verify::isValidSoftApPassword("~~~~~~~~"));
}

TEST_CASE("control DEL and high-bit bytes are rejected", "[verify]")
{
	const std::array<char, 8> belowPrintable{'a', 'b', 'c', 'd', 'e', 'f', 'g', static_cast<char>(0x1F)};
	const std::array<char, 8> deleteCharacter{'a', 'b', 'c', 'd', 'e', 'f', 'g', static_cast<char>(0x7F)};
	const std::array<char, 8> highBitCharacter{'a', 'b', 'c', 'd', 'e', 'f', 'g', static_cast<char>(0x80)};
	const std::array<char, 9> utf8Password{
		'p', 'a', 's', 's', 'w', static_cast<char>(0xC3), static_cast<char>(0xB6), 'r', 'd',
	};

	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string_view(belowPrintable.data(), belowPrintable.size())));
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string_view(deleteCharacter.data(), deleteCharacter.size())));
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string_view(highBitCharacter.data(), highBitCharacter.size())));
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string_view(utf8Password.data(), utf8Password.size())));
}

TEST_CASE("embedded NUL is rejected across the complete string view", "[verify]")
{
	const std::array<char, 9> password{'a', 'b', 'c', 'd', '\0', 'e', 'f', 'g', 'h'};

	TEST_ASSERT_FALSE(verify::isValidSoftApPassword(std::string_view(password.data(), password.size())));
}

TEST_CASE("64 character hexadecimal PSK remains unsupported", "[verify]")
{
	TEST_ASSERT_FALSE(verify::isValidSoftApPassword("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
}