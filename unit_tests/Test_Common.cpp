#include "Test_Common.hpp"
#include "Utils.hpp"
#include <sstream>
#include <iostream>

void TestBase::PrintBuffer(std::string label, const std::vector<uint8_t>& buffer, uint32_t line_len)
{
	assert(line_len > 0);

	std::stringstream ss;
	std::vector<char> hex(line_len * 3);
	uint32_t offset = 0;
	uint32_t len_remaining = buffer.size();
	const uint8_t* buffer_p = buffer.data();

	ss << label << ":" << std::endl;
	
	while (len_remaining)
	{
		uint32_t len = std::min(line_len, len_remaining);
		auto hex_chars = BinToHexString(buffer_p, len, hex.data(), hex.size()) * 3;
		ss << "   " << std::string(hex.data(), hex.data() + hex_chars) << std::endl;
		len_remaining -= len;
		buffer_p += len;
	}

	std::cout << ss.rdbuf();
}