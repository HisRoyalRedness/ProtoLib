/*
	PDU unit tests

	Keith Fletcher
	June 2021

	This file is Unlicensed.
	See the foot of the file, or refer to <http://unlicense.org>
*/

#include "ProtoPdu.hpp"
#include "Diagnostics.hpp"
#include "Test_Common.hpp"

using testing::ElementsAre;

class Pdu_Test : public TestBase
{
public:
	static constexpr size_t PDU_SIZE = 50;
	static constexpr size_t PDU_COUNT = 2;

	// Put this line back in if you want to see the diagnostic output
	//PduAllocator_Test() : m_pdu_alloc(m_diag) {}

protected:
	void SetUp() override
	{
		// Allocate a fresh pdu
		m_pdu = m_pdu_alloc.Allocate();
	}

	void TearDown() override
	{
		// Free up the PDU
		m_pdu = nullptr;
	}

	Diagnostics m_diag;
	PduAllocator<PDU_SIZE, PDU_COUNT> m_pdu_alloc;
	PduPtr m_pdu;
};




TEST_F(Pdu_Test, PutDownIsSequentialWithCursor)
{
	m_pdu->PutDown((uint32_t)0x01020304);
	m_pdu->PutDown((uint16_t)0x0506U);
	m_pdu->PutDown((uint8_t)0x07U);

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + 7),
		ElementsAre(
			0x01, 0x02, 0x03, 0x04,
			0x05, 0x06,
			0x07));	// The implied trailing 0
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