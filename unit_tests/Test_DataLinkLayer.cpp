/*
	DatalinkLayer unit tests

	Keith Fletcher
	May 2021

	This file is Unlicensed.
	See the foot of the file, or refer to <http://unlicense.org>
*/

#include "DataLinkLayer.hpp"
#include "DleEncoder.hpp"
#include "CRC32_software.hpp"
#include "Test_Common.hpp"
#include "Diagnostics.hpp"


using testing::ElementsAre;

class DatalinkLayer_Test : public TestBase
{
public:
	static constexpr size_t PDU_SIZE = 255;
	static constexpr size_t PDU_COUNT = 5;
	static constexpr uint8_t DECODED_DATA[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 
		0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14 };
	static constexpr uint8_t ENCODED_DATA[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 
		0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 
		0x14, 0x19, 0x58, 0x81, 0xFE };
	static constexpr uint8_t DATA_CRC[] = { 0x19, 0x58, 0x81, 0xFE };

	// Put this line back in if you want to see the diagnostic output
	//DatalinkLayer_Test() : m_pdu_alloc(m_diag) {}

protected:
	//void SetUp() override
	//{
	//}

	//void TearDown() override
	//{
	//}

	DatalinkLayer< DleEncoder, CRC32_SW> m_dl_layer_dle;
	Diagnostics m_diag;
	PduAllocator<PDU_SIZE, PDU_COUNT> m_pdu_alloc;
};

TEST_F(DatalinkLayer_Test, EncodingBasicData)
{
	// Allocate a fresh pdu and populate
	auto pdu = m_pdu_alloc.Allocate(sizeof(DECODED_DATA));
	for (int i = 0; i < sizeof(DECODED_DATA); ++i)
		(pdu->Data()[i]) = DECODED_DATA[i];

	// Encode the data
	auto pdu_out = m_dl_layer_dle.Encode(std::move(pdu));

	// Check the output
	ASSERT_NE(nullptr, pdu_out);
	ASSERT_THAT(
		std::vector<uint8_t>(pdu_out->Data(), pdu_out->Data() + pdu_out->GetDataLen()),
		ElementsAre(
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
			0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
			0x19, 0x58, 0x81, 0xfe)); // CRC
}

TEST_F(DatalinkLayer_Test, DecodingBasicData)
{
	// Allocate a fresh pdu and populate
	auto pdu = m_pdu_alloc.Allocate(sizeof(ENCODED_DATA));
	for (int i = 0; i < sizeof(ENCODED_DATA); ++i)
		(pdu->Data()[i]) = ENCODED_DATA[i];

	// Encode the data
	auto pdu_out = m_dl_layer_dle.Decode(std::move(pdu));

	ASSERT_NE(nullptr, pdu_out);
	ASSERT_THAT(
		std::vector<uint8_t>(pdu_out->Data(), pdu_out->Data() + pdu_out->GetDataLen()),
		ElementsAre(
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
			0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14
			)); // NO CRC
}





/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/