#pragma once

#include "Test_Common.hpp"

template<typename Encoder, bool BufferPrintingDefault>
class TestBase_Encoder : public TestBase
{
public:
	template<typename T>
	std::vector<T> ToVec(std::initializer_list<T> init) const { return std::vector<T>(init); }

	std::vector<uint8_t> ToVec8(std::initializer_list<uint8_t> init) const { return ToVec(init); }

protected:

	std::vector<uint8_t> Encode(std::initializer_list<uint8_t> source_list, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault)
	{
		return Encode(std::vector<uint8_t>(source_list), print_source, print_target);
	}

	std::vector<uint8_t> Encode(std::vector<uint8_t> source_list, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault)
	{
		if (print_source)
			PrintBuffer("Clear", source_list);

		std::vector<uint8_t> target(_encoder.MaxEncodeLen(source_list.size()));
		EncodeResult result = _encoder.Encode(source_list.data(), source_list.size(), target.data(), target.size());

		// Make sure bytes read and written aren't more than they should be
		assert(result.BytesRead <= source_list.size());
		assert(result.BytesWritten <= target.size());

		if (result.BytesWritten == 0)
			target.clear();
		else if (result.BytesWritten != target.size())
			target.resize(result.BytesWritten);

		if (print_target)
			PrintBuffer("Encoded", target);

		return target;
	}

	std::vector<uint8_t> Decode(std::initializer_list<uint8_t> source_list, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault)
	{
		return Decode(std::vector<uint8_t>(source_list), print_source, print_target);
	}

	std::vector<uint8_t> Decode(std::vector<uint8_t> source_list, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault)
	{
		if (print_source)
			PrintBuffer("Encoded", source_list);

		std::vector<uint8_t> target(_encoder.MaxDecodeLen(source_list.size())); 
		EncodeResult result = _encoder.Decode(source_list.data(), source_list.size(), target.data(), target.size());

		// Make sure bytes read and written aren't more than they should be
		assert(result.BytesRead <= source_list.size());
		assert(result.BytesWritten <= target.size());

		if (result.BytesWritten == 0)
			target.clear();
		else if (result.BytesWritten != target.size())
			target.resize(result.BytesWritten);

		if (print_target)
			PrintBuffer("Decoded", target);

		return target;
	}

	Encoder _encoder;
};