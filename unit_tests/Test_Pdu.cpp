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

#pragma region Test class
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
		ASSERT_EQ(0, TypedPdu()->GetReadCursor());
		ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
	}

	void TearDown() override
	{
		// Free up the PDU
		m_pdu = nullptr;
	}

	void PopulatePdu()
	{
		static constexpr uint8_t DATA[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };
		for (int i = 0; i < sizeof(DATA); ++i)
			m_pdu->WriteableData()[i] = DATA[i];
		m_pdu->SetDataLen(sizeof(DATA));
	}

	const ProtoPdu<PDU_SIZE>* TypedPdu() const { return reinterpret_cast<ProtoPdu<PDU_SIZE>*>(&*m_pdu); }

	Diagnostics m_diag;
	PduAllocator<PDU_SIZE, PDU_COUNT> m_pdu_alloc;
	PduPtr m_pdu;

};
#pragma endregion Test class

#pragma region PutDown tests
TEST_F(Pdu_Test, PutDownUInt8)
{
	constexpr auto data = 0x12ui8;
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownUInt16)
{
	constexpr auto data = 0x1234ui16;
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownUInt32)
{
	constexpr auto data = 0x12345678ui32;
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownUInt64)
{
	constexpr auto data = 0x1234567801020304ui64;
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78,
			0x01, 0x02, 0x03, 0x04, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownInt64_GeneralCase)
{
	// The template specialisation for this type has been excluded in
	// the unit test build so that the general template case can be tested
	constexpr auto data = 0x1234567801020304i64;
	ASSERT_TRUE(m_pdu->PutDown(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78,
			0x01, 0x02, 0x03, 0x04, 0x00));
	ASSERT_EQ(sizeof(data), TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownIsSequentialWithCursor)
{
	m_pdu->PutDown(0x1122334455667788ui64);
	m_pdu->PutDown(0x01020304ui32);
	m_pdu->PutDown(0x0506ui16);
	m_pdu->PutDown(0x07ui8);

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + 15),
		ElementsAre(
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
			0x01, 0x02, 0x03, 0x04,
			0x05, 0x06,
			0x07));
}

TEST_F(Pdu_Test, PutToTheMaximum)
{
	uint8_t data[PDU_SIZE];
	for (int i = 0; i < PDU_SIZE; ++i)
		data[i] = i;

	ASSERT_TRUE(m_pdu->PutDown(&data[0], sizeof(data)));
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutPastTheMaximum)
{
	uint8_t data[PDU_SIZE];
	for (int i = 0; i < PDU_SIZE; ++i)
		data[i] = i;

	ASSERT_TRUE(m_pdu->PutDown(&data[0], sizeof(data)));
	ASSERT_DEATH({ m_pdu->PutDown((uint8_t)1); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetWriteCursor());
}
#pragma endregion PutDown tests

#pragma region PutDownRev tests
TEST_F(Pdu_Test, PutDownRevUInt8)
{
	constexpr auto data = 0x12ui8;
	m_pdu->SkipWrite(sizeof(data));
	ASSERT_TRUE(m_pdu->PutDownRev(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x00));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownRevUInt16)
{
	constexpr auto data = 0x1234ui16;
	m_pdu->SkipWrite(sizeof(data));
	ASSERT_TRUE(m_pdu->PutDownRev(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x00));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownRevUInt32)
{
	constexpr auto data = 0x12345678ui32;
	m_pdu->SkipWrite(sizeof(data));
	ASSERT_TRUE(m_pdu->PutDownRev(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78, 0x00));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownRevUInt64)
{
	constexpr auto data = 0x123456789abcdef0ui64;
	m_pdu->SkipWrite(sizeof(data));
	ASSERT_TRUE(m_pdu->PutDownRev(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownRevInt64_GeneralCase)
{
	// The template specialisation for this type has been excluded in
	// the unit test build so that the general template case can be tested
	constexpr auto data = 0x123456789abcdef0i64;
	m_pdu->SkipWrite(sizeof(data));
	ASSERT_TRUE(m_pdu->PutDownRev(data));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + sizeof(data) + 1),
		ElementsAre(
			0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutDownRevIsSequentialWithCursor)
{
	m_pdu->SkipWrite(15);
	m_pdu->PutDownRev(0x1122334455667788ui64);
	m_pdu->PutDownRev(0x01020304ui32);
	m_pdu->PutDownRev(0x0506ui16);
	m_pdu->PutDownRev(0x07ui8);

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + 15),
		ElementsAre(
			0x07,
			0x05, 0x06,
			0x01, 0x02, 0x03, 0x04,
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88));
}

TEST_F(Pdu_Test, PutRevToTheMimimum)
{
	m_pdu->SkipWrite(1);
	ASSERT_TRUE(m_pdu->PutDownRev(0x01ui8));
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, PutRevPastTheMinimum)
{
	m_pdu->SkipWrite(1);
	ASSERT_TRUE(m_pdu->PutDownRev(0x01ui8));
	ASSERT_DEATH({ m_pdu->PutDownRev(0x01ui8); }, "Assertion failed");
	ASSERT_EQ(0, TypedPdu()->GetWriteCursor());
}
#pragma endregion PutDownRev tests

#pragma region Skip tests
TEST_F(Pdu_Test, SkipReadIncreasesTheCursor)
{
	PopulatePdu();
	uint64_t data64 = 0;
	uint32_t data32 = 0;
	uint16_t data16 = 0;
	uint8_t data8 = 0;
	// 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90
	ASSERT_TRUE(m_pdu->PickUp(data64));
	ASSERT_EQ(0x0123456789abcdefui64, data64);
	ASSERT_EQ(8, m_pdu->GetReadCursor());

	ASSERT_TRUE(m_pdu->PickUp(data32));
	ASSERT_EQ(0x10203040ui32, data32);
	ASSERT_EQ(12, m_pdu->GetReadCursor());

	ASSERT_TRUE(m_pdu->PickUp(data16));
	ASSERT_EQ(0x5060ui16, data16);
	ASSERT_EQ(14, m_pdu->GetReadCursor());

	ASSERT_TRUE(m_pdu->PickUp(data8));
	ASSERT_EQ(0x70ui8, data8);
	ASSERT_EQ(15, m_pdu->GetReadCursor());
}

TEST_F(Pdu_Test, SkipReadToTheMaximum)
{
	uint8_t data = 0;
	ASSERT_TRUE(m_pdu->SkipRead(PDU_SIZE - 1));
	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetReadCursor());
}

TEST_F(Pdu_Test, SkipReadPastTheMaximum)
{
	uint8_t data = 0;
	ASSERT_TRUE(m_pdu->SkipRead(PDU_SIZE));
	ASSERT_DEATH({ m_pdu->PickUp(data); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetReadCursor());
}

TEST_F(Pdu_Test, SkipReadToTheMaximumDataLen)
{
	PopulatePdu();
	uint8_t data = 0;
	ASSERT_TRUE(m_pdu->SkipRead(m_pdu->GetDataLen() - 1));
	ASSERT_TRUE(m_pdu->PickUp(data));
	ASSERT_EQ(0x90, data);
	ASSERT_EQ(m_pdu->GetDataLen(), TypedPdu()->GetReadCursor());
}

TEST_F(Pdu_Test, SkipReadPastTheMaximumDataLen)
{
	PopulatePdu();
	ASSERT_DEATH({ m_pdu->SkipRead(m_pdu->GetDataLen() + 1); }, "Assertion failed");
}

TEST_F(Pdu_Test, SkipWriteIncreasesTheCursor)
{
	ASSERT_TRUE(m_pdu->SkipWrite(1));
	ASSERT_TRUE(m_pdu->PutDown(0x12ui8));
	ASSERT_TRUE(m_pdu->SkipWrite(2));
	ASSERT_TRUE(m_pdu->PutDown(0x3456ui16));

	ASSERT_THAT(
		std::vector<uint8_t>(m_pdu->Data(), m_pdu->Data() + 7),
		ElementsAre(
			0x00, 0x12, 0x00, 0x00, 0x34, 0x56, 0x00));
	ASSERT_EQ(6, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, SkipWriteToTheMaximum)
{
	ASSERT_TRUE(m_pdu->SkipWrite(PDU_SIZE));
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetWriteCursor());
}

TEST_F(Pdu_Test, SkipWritePastTheMaximum)
{
	ASSERT_TRUE(m_pdu->SkipWrite(PDU_SIZE));
	ASSERT_DEATH({ m_pdu->SkipWrite(1); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetWriteCursor());
}
#pragma endregion Skip tests

#pragma region Pickup tests
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

TEST_F(Pdu_Test, PickUpIsSequentialWithCursor)
{
	PopulatePdu();
	uint64_t data64 = 0;
	uint32_t data32 = 0;
	uint16_t data16 = 0;
	uint8_t data8 = 0;
	// 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90
	ASSERT_TRUE(m_pdu->PickUp(data64));
	ASSERT_EQ(0x0123456789abcdefui64, data64);

	ASSERT_TRUE(m_pdu->PickUp(data32));
	ASSERT_EQ(0x10203040ui32, data32);

	ASSERT_TRUE(m_pdu->PickUp(data16));
	ASSERT_EQ(0x5060ui16, data16);

	ASSERT_TRUE(m_pdu->PickUp(data8));
	ASSERT_EQ(0x70ui8, data8);
}

TEST_F(Pdu_Test, PickToTheMaximum)
{
	uint8_t data;
	for (int i = 0; i < PDU_SIZE; ++i)
		ASSERT_TRUE(m_pdu->PickUp(data));

	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetReadCursor());
}

TEST_F(Pdu_Test, PickPastTheMaximum)
{
	uint8_t data;
	for (int i = 0; i < PDU_SIZE; ++i)
		ASSERT_TRUE(m_pdu->PickUp(data));

	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetReadCursor());
	ASSERT_DEATH({ m_pdu->PickUp(data); }, "Assertion failed");
	ASSERT_EQ(PDU_SIZE, TypedPdu()->GetReadCursor());
}

#pragma endregion Pickup tests

#pragma region PickupRev tests
TEST_F(Pdu_Test, PickUpRevUInt8)
{
	PopulatePdu();
	uint8_t data = 0;
	m_pdu->SkipRead(sizeof(data));

	ASSERT_TRUE(m_pdu->PickUpRev(data));
	ASSERT_EQ(data, 0x01);
}

TEST_F(Pdu_Test, PickUpRevUInt16)
{
	PopulatePdu();
	uint16_t data = 0;
	m_pdu->SkipRead(sizeof(data));

	ASSERT_TRUE(m_pdu->PickUpRev(data));
	ASSERT_EQ(data, 0x0123);
}

TEST_F(Pdu_Test, PickUpRevUInt32)
{
	PopulatePdu();
	uint32_t data = 0;
	m_pdu->SkipRead(sizeof(data));

	ASSERT_TRUE(m_pdu->PickUpRev(data));
	ASSERT_EQ(data, 0x01234567);
}

TEST_F(Pdu_Test, PickUpRevUInt64)
{
	PopulatePdu();
	uint64_t data = 0;
	m_pdu->SkipRead(sizeof(data));

	ASSERT_TRUE(m_pdu->PickUpRev(data));
	ASSERT_EQ(data, 0x0123456789abcdef);
}

TEST_F(Pdu_Test, PickUpRevIsSequentialWithCursor)
{
	PopulatePdu();
	uint64_t data64 = 0;
	uint32_t data32 = 0;
	uint16_t data16 = 0;
	uint8_t data8 = 0;
	m_pdu->SkipRead(sizeof(data8) + sizeof(data16) + sizeof(data32) + sizeof(data64));

	// 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90

	ASSERT_TRUE(m_pdu->PickUpRev(data64));
	ASSERT_EQ(0xef10203040506070ui64, data64);

	ASSERT_TRUE(m_pdu->PickUpRev(data32));
	ASSERT_EQ(0x6789abcdui32, data32);

	ASSERT_TRUE(m_pdu->PickUpRev(data16));
	ASSERT_EQ(0x2345ui16, data16);

	ASSERT_TRUE(m_pdu->PickUpRev(data8));
	ASSERT_EQ(0x01ui8, data8);
}

TEST_F(Pdu_Test, PickRevFromTheMaximum)
{
	PopulatePdu();
	uint8_t data;
	ASSERT_TRUE(m_pdu->SkipRead(m_pdu->GetDataLen()));
	ASSERT_TRUE(m_pdu->PickUpRev(data));

	ASSERT_EQ(0x90, data);
}

TEST_F(Pdu_Test, PickRevToTheMinimum)
{
	PopulatePdu();
	uint8_t data;
	m_pdu->SkipRead(1);
	ASSERT_TRUE(m_pdu->PickUpRev(data));

	ASSERT_EQ(0, TypedPdu()->GetReadCursor());
}

TEST_F(Pdu_Test, PickRevPastTheMinimum)
{
	PopulatePdu();
	uint8_t data;
	m_pdu->SkipRead(1);
	ASSERT_TRUE(m_pdu->PickUpRev(data));

	ASSERT_EQ(0, TypedPdu()->GetReadCursor());
	ASSERT_DEATH({ m_pdu->PickUpRev(data); }, "Assertion failed");
	ASSERT_EQ(0, TypedPdu()->GetReadCursor());
}

#pragma endregion PickupRev tests

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