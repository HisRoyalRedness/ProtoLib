#pragma once

#include "Test_Common.hpp"
#include <functional>

template<typename Encoder, bool BufferPrintingDefault>
class TestBase_Encoder : public TestBase
{
public:
	template<typename T>
	std::vector<T> ToVec(std::initializer_list<T> init) const { return std::vector<T>(init); }

	std::vector<uint8_t> ToVec8(std::initializer_list<uint8_t> init) const { return ToVec(init); }

protected:

	void DecodeAnErrorPacket(std::initializer_list<uint8_t> init, std::initializer_list<uint8_t> expected, bool check_bytes_read = true, bool check_bytes_written = true);
	void DecodeAnErrorPacket(const std::vector<uint8_t>& source, const std::vector<uint8_t>& expected, bool check_bytes_read = true, bool check_bytes_written = true);

	std::vector<uint8_t> Encode(const std::initializer_list<uint8_t> source, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault);
	std::vector<uint8_t> Encode(const std::vector<uint8_t>& source, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault);
	std::vector<uint8_t> Decode(const std::initializer_list<uint8_t> source, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault);
	std::vector<uint8_t> Decode(const std::vector<uint8_t>& source, bool print_source = BufferPrintingDefault, bool print_target = BufferPrintingDefault);

	Encoder _encoder;
};


//---------------------------------------------------------------------------------------------------------------------------
// Implementation

#define TEST_ASSERT(test, msg) if (!(test)) { ::testing::internal::AssertHelper(::testing::TestPartResult::kFatalFailure, __FILE__, __LINE__, msg) = ::testing::Message(); }

template<typename Encoder, bool BufferPrintingDefault>
std::vector<uint8_t> TestBase_Encoder<Encoder, BufferPrintingDefault>::Encode(std::initializer_list<uint8_t> source, bool print_source, bool print_target)
{
	return Encode(std::vector<uint8_t>(source), print_source, print_target);
}

template<typename Encoder, bool BufferPrintingDefault>
std::vector<uint8_t> TestBase_Encoder<Encoder, BufferPrintingDefault>::Encode(const std::vector<uint8_t>& source, bool print_source, bool print_target)
{
	TEST_ASSERT(source.size() > 0, "Empty source vector");

	if (print_source)
		PrintBuffer("Clear", source);

	std::vector<uint8_t> target(_encoder.MaxEncodeLen(source.size()));
	EncodeResult result = _encoder.Encode(source.data(), source.size(), target.data(), target.size());

	// Make sure bytes read and written aren't more than they should be
	TEST_ASSERT(result.BytesRead <= source.size(), "Too many bytes read");
	TEST_ASSERT(result.BytesWritten <= target.size(), "Too many bytes written");
	TEST_ASSERT(!result.Error, "Encoding error");

	if (result.BytesWritten == 0)
		target.clear();
	else if (result.BytesWritten != target.size())
		target.resize(result.BytesWritten);

	if (print_target)
		PrintBuffer("Encoded", target);

	return target;
}

template<typename Encoder, bool BufferPrintingDefault>
std::vector<uint8_t> TestBase_Encoder<Encoder, BufferPrintingDefault>::Decode(const std::initializer_list<uint8_t> source, bool print_source, bool print_target)
{
	return Decode(std::vector<uint8_t>(source), print_source, print_target);
}

template<typename Encoder, bool BufferPrintingDefault>
std::vector<uint8_t> TestBase_Encoder<Encoder, BufferPrintingDefault>::Decode(const std::vector<uint8_t>& source, bool print_source, bool print_target)
{
	TEST_ASSERT(source.size() > 0, "Empty source vector");

	if (print_source)
		PrintBuffer("Encoded", source);

	std::vector<uint8_t> target(_encoder.MaxDecodeLen(source.size()));
	EncodeResult result = _encoder.Decode(source.data(), source.size(), target.data(), target.size());

	// Make sure bytes read and written aren't more than they should be
	TEST_ASSERT(result.BytesRead <= source.size(), "Too many bytes read");
	TEST_ASSERT(result.BytesWritten <= target.size(), "Too many bytes written");
	TEST_ASSERT(!result.Error, "Decoding error");

	if (result.BytesWritten == 0)
		target.clear();
	else if (result.BytesWritten != target.size())
		target.resize(result.BytesWritten);

	if (print_target)
		PrintBuffer("Decoded", target);

	return target;
}

template<typename Encoder, bool BufferPrintingDefault>
void TestBase_Encoder<Encoder, BufferPrintingDefault>::DecodeAnErrorPacket(std::initializer_list<uint8_t> init, std::initializer_list<uint8_t> expected, bool check_bytes_read, bool check_bytes_written)
{
	DecodeAnErrorPacket(std::vector<uint8_t>(init), std::vector<uint8_t>(expected), check_bytes_read, check_bytes_written);
}

template<typename Encoder, bool BufferPrintingDefault>
void TestBase_Encoder<Encoder, BufferPrintingDefault>::DecodeAnErrorPacket(const std::vector<uint8_t>& source, const std::vector<uint8_t>& expected, bool check_bytes_read, bool check_bytes_written)
{
	// Assumes that source contains valid data, followed by the error characters
	std::vector<uint8_t> target(_encoder.MaxDecodeLen(source.size()));

	EncodeResult result = _encoder.Decode(source.data(), source.size(), target.data(), target.size());

	if (check_bytes_read)
		ASSERT_THAT(result.BytesRead, expected.size());
	if (check_bytes_written)
		ASSERT_THAT(result.BytesWritten, expected.size());
	ASSERT_TRUE(result.Error);

	target.resize(expected.size());
	EXPECT_THAT(target, expected);
}