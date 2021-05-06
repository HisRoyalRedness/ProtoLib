#include "Test_CommonEncoder.hpp"
#include "DleEncoder.hpp"
#include <list>
#include <vector>
#include <algorithm>

using testing::ElementsAre;

class DleEncoderTest : public TestBase_Encoder<DleEncoder, false> // Set to true to print buffers by default
{

};


TEST_F(DleEncoderTest, EncodingReservedCharacters)
{
	EXPECT_THAT(Encode({ DleEncoder::STX }), ElementsAre(DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::STX));
	EXPECT_THAT(Encode({ DleEncoder::ETX }), ElementsAre(DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::ETX));
	EXPECT_THAT(Encode({ DleEncoder::DLE }), ElementsAre(DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::DLE));
}

TEST_F(DleEncoderTest, DecodingReservedCharacters)
{
	EXPECT_THAT(Decode({ DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::STX }), ElementsAre(DleEncoder::STX));
	EXPECT_THAT(Decode({ DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::ETX }), ElementsAre(DleEncoder::ETX));
	EXPECT_THAT(Decode({ DleEncoder::DLE, DleEncoder::DLE ^ DleEncoder::DLE }), ElementsAre(DleEncoder::DLE));
}

TEST_F(DleEncoderTest, EncodingNonReservedCharacters)
{
	std::vector<uint8_t> source;
	for (int i = 0; i < 256; i++)
	{
		if (i != DleEncoder::STX && i != DleEncoder::ETX && i != DleEncoder::DLE)
			source.push_back(static_cast<uint8_t>(i));
	}

	EXPECT_THAT(Encode(source), source);
}

TEST_F(DleEncoderTest, DecodingNonReservedCharacters)
{
	std::vector<uint8_t> source;
	for (int i = 0; i < 256; i++)
	{
		if (i != DleEncoder::STX && i != DleEncoder::ETX && i != DleEncoder::DLE)
			source.push_back(static_cast<uint8_t>(i));
	}

	EXPECT_THAT(Decode(source), source);
}

TEST_F(DleEncoderTest, DecodeAPacketEndingWithDle)
{
	std::vector<uint8_t> source({ 0x20, 0x21, DleEncoder::DLE });
	std::vector<uint8_t> target(_encoder.MaxDecodeLen(source.size()));

	EncodeResult result = _encoder.Decode(source.data(), source.size(), target.data(), target.size());

	ASSERT_THAT(result.BytesRead, 2); // Should only consume the valid bytes
	ASSERT_THAT(result.BytesWritten, 2); 

	target.resize(2);
	EXPECT_THAT(target, ElementsAre(0x20, 0x21));
}

TEST_F(DleEncoderTest, DecodeAPacketContainingReservedCharacters)
{
	DecodeAnErrorPacket({ 0x01, DleEncoder::STX }, { 0x01 });
	DecodeAnErrorPacket({ 0x01, DleEncoder::ETX }, { 0x01 });
}

TEST_F(DleEncoderTest, DecodeAPacketContainingEscapedReservedCharacters)
{
	DecodeAnErrorPacket({ 0x01, DleEncoder::DLE, DleEncoder::STX }, { 0x01 });
	DecodeAnErrorPacket({ 0x01, DleEncoder::DLE, DleEncoder::ETX }, { 0x01 });
	DecodeAnErrorPacket({ 0x01, DleEncoder::DLE, DleEncoder::DLE }, { 0x01 });
}

