#include "Test_CommonEncoder.hpp"
#include "CobsEncoder.hpp"
#include <list>
#include <vector>

using testing::ElementsAre;

class CobsEncoderTest : public TestBase_Encoder<CobsEncoder, false> // Set to true to print buffers by default
{

};

// A minimal buffer consisting of only a zero
TEST_F(CobsEncoderTest, MaxPacketSizeIs254Bytes)
{
	ASSERT_EQ(CobsEncoder::MAX_PACKET_SIZE, 254);
}

// A minimal buffer consisting of only a zero
TEST_F(CobsEncoderTest, EncodeAMinimalZeroBuffer) 
{
	ASSERT_THAT(
		Encode({ 0x00 }), 
		ElementsAre(
			0x01,	// The first 0 that is part of the data
			0x01));	// The implied trailing 0

	EXPECT_THAT(
		Decode({ 0x01, 0x01 }, false),
		ElementsAre(0x00));	// The implied trailing 0
}

// A minimal buffer consisting of a single non-zero byte
TEST_F(CobsEncoderTest, EncodeAMinimalNonZeroBuffer)
{
	ASSERT_THAT(
		Encode({ 0x09 }), 
		ElementsAre(
			0x02,	// Two bytes (including the trailing implied 0)
			0x09));	// The 1 non-zero byte

	EXPECT_THAT(
		Decode({ 0x02, 0x09 }, false),
		ElementsAre(0x09));
}

// Basic sanity check to make sure encode and decode work as expected
TEST_F(CobsEncoderTest, EncodeASmallBufferRoundTrip)
{
	ASSERT_THAT(
		Encode({ 0x01, 0x02, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }), 
		ToVec8({ 0x03, 0x01, 0x02, 0x04, 0x01, 0x02, 0x03, 0x05, 0x01, 0x02, 0x03, 0x04, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05 }));

	EXPECT_THAT(
		Decode({ 0x03, 0x01, 0x02, 0x04, 0x01, 0x02, 0x03, 0x05, 0x01, 0x02, 0x03, 0x04, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05 }, false),
		ToVec8({ 0x01, 0x02, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }));
}

TEST_F(CobsEncoderTest, EncodePacketAtMaximumLength)
{
	//Clear:
	//   01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
	//   21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40
	//   41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60
	//   61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F 80
	//   81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
	//   A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF C0
	//   C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF E0
	//   E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE
	std::vector<uint8_t> buffer_clear(CobsEncoder::MAX_PACKET_SIZE);
	for (int i = 0; i < buffer_clear.size(); ++i)
		buffer_clear[i] = static_cast<uint8_t>(i + 1);

	//Encoded:
	//   FF 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
	//   20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
	//   40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F
	//   60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F
	//   80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F
	//   A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF
	//   C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF
	//   E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE
	std::vector<uint8_t> buffer_encoded(CobsEncoder::MAX_PACKET_SIZE + 1);
	buffer_encoded[0] = 0xff;
	for (int i = 0; i < buffer_encoded.size() - 1; ++i)
		buffer_encoded[i + 1] = static_cast<uint8_t>(i + 1);
	

	ASSERT_THAT(Encode(buffer_clear), buffer_encoded);
	EXPECT_THAT(Decode(buffer_encoded, false), buffer_clear);
}

TEST_F(CobsEncoderTest, EncodePacketPastMaximumLength)
{
	//Clear:
	//   01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
	//   21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40
	//   41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60
	//   61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F 80
	//   81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
	//   A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF C0
	//   C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF E0
	//   E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF 00
	//   01 02 03 04 05 06 07 08 09 0A 0B 0C 0D
	static const uint8_t extra = 15;
	std::vector<uint8_t> buffer_clear(CobsEncoder::MAX_PACKET_SIZE + extra);
	for (int i = 0; i < buffer_clear.size(); ++i)
		buffer_clear[i] = static_cast<uint8_t>(i + 1);

	// Expect a 255-byte buffer, with the first value being 0xff
	//Encoded:
	//   FF 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
	//   20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
	//   40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F
	//   60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F
	//   80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F
	//   A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF
	//   C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF
	//   E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE 02
	//   FF 0E 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D
	std::vector<uint8_t> buffer_encoded(CobsEncoder::MAX_PACKET_SIZE + extra + 2);
	int i = 0;
	buffer_encoded[i++] = 0xff;
	for (; i < CobsEncoder::MAX_PACKET_SIZE + 1; ++i)
		buffer_encoded[i] = static_cast<uint8_t>(i); // Close off the first data packet
	// The first 0 after the close off
	buffer_encoded[i++] = 0x02;
	buffer_encoded[i++] = 0xff; 
	// The rest
	buffer_encoded[i++] = extra - 1;
	for (int j = 0; j < extra - 2; ++j)
		buffer_encoded[i + j] = static_cast<uint8_t>(j + 1); 


	EXPECT_THAT(Encode(buffer_clear), buffer_encoded);
	EXPECT_THAT(Decode(buffer_encoded), buffer_clear);
}

