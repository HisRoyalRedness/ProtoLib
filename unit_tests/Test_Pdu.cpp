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
		ASSERT_EQ(0, TypedPdu()->GetCursor());
	}

	void TearDown() override
	{
		// Free up the PDU
		m_pdu = nullptr;
	}

	void PopulatePdu()
	{
		static constexpr uint8_t DATA[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x10, 0x20, 0x30, 0x40, 0x50 };
		for (int i = 0; i < sizeof(DATA); ++i)
			m_pdu->Data()[i] = DATA[i];
	}

	const ProtoPdu<PDU_SIZE>* TypedPdu() const { return reinterpret_cast<ProtoPdu<PDU_SIZE>*>(&*m_pdu); }

	Diagnostics m_diag;
	PduAllocator<PDU_SIZE, PDU_COUNT> m_pdu_alloc;
	PduPtr m_pdu;

};

TEST_F(Pdu_Test, PutDownUInt8)
{
	constexpr auto data = static_cast<uint8_t>(0x12UL);
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PutDownUInt16)
{
	constexpr auto data = static_cast<uint16_t>(0x1234UL);
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PutDownUInt32)
{
	constexpr auto data = static_cast<uint32_t>(0x12345678UL);
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PutDownUInt64)
{
	constexpr auto data = static_cast<uint64_t>(0x1234567801020304ULL);
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78,
			0x01, 0x02, 0x03, 0x04, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, SkipIncreasesTheCursor)
{
	ASSERT_TRUE(m_pdu->Skip(1));
	ASSERT_TRUE(m_pdu->PutDown((uint8_t)0x12));
	ASSERT_TRUE(m_pdu->Skip(2));
	ASSERT_TRUE(m_pdu->PutDown((uint16_t)0x3456));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + 7),
		ElementsAre(
			0x00, 0x12, 0x00, 0x00, 0x34, 0x56, 0x00));
	ASSERT_EQ(6, TypedPdu()->GetCursor());
}

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
			0x07));
}

TEST_F(Pdu_Test, SkipToTheMaximum)
{
	ASSERT_TRUE(m_pdu->Skip(PDU_SIZE));
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, SkipPastTheMaximum)
{
	ASSERT_TRUE(m_pdu->Skip(PDU_SIZE));
	ASSERT_DEATH({ m_pdu->Skip(1); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PutToTheMaximum)
{
	uint8_t data[PDU_SIZE];
	for (int i = 0; i < PDU_SIZE; ++i)
		data[i] = i;

	ASSERT_TRUE(m_pdu->PutDown(&data[0], sizeof(data)));
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PutPastTheMaximum)
{
	uint8_t data[PDU_SIZE];
	for (int i = 0; i < PDU_SIZE; ++i)
		data[i] = i;

	ASSERT_TRUE(m_pdu->PutDown(&data[0], sizeof(data)));
	ASSERT_DEATH({ m_pdu->PutDown((uint8_t)1); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PickUpUInt8)
{
	PopulatePdu();
	uint8_t data = 0;

	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(data, 0x01);
}

TEST_F(Pdu_Test, PickUpUInt16)
{
	PopulatePdu();
	uint16_t data = 0;

	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(data, 0x0123);
}

TEST_F(Pdu_Test, PickUpUInt32)
{
	PopulatePdu();
	uint32_t data = 0;

	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(data, 0x01234567);
}

TEST_F(Pdu_Test, PickUpUInt64)
{
	PopulatePdu();
	uint64_t data = 0;

	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(data, 0x0123456789abcdef);
}

TEST_F(Pdu_Test, PickToTheMaximum)
{
	uint8_t data;
	for (int i = 0; i < PDU_SIZE; ++i)
		ASSERT_TRUE(m_pdu->PickUp(data));

	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
}

TEST_F(Pdu_Test, PickPastTheMaximum)
{
	uint8_t data;
	for (int i = 0; i < PDU_SIZE; ++i)
		ASSERT_TRUE(m_pdu->PickUp(data));

	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
	ASSERT_DEATH({ m_pdu->PickUp(data); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetCursor());
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