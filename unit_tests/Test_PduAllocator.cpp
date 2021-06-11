/*
	PDU allocation unit tests

	Keith Fletcher
	June 2021

	This file is Unlicensed.
	See the foot of the file, or refer to <http://unlicense.org>
*/

#include "ProtoPdu.hpp"
#include "Diagnostics.hpp"
#include "Test_Common.hpp"

using testing::ElementsAre;

class PduAllocator_Test : public TestBase
{
public:
	static constexpr size_t PDU_SIZE = 10;
	static constexpr size_t PDU_COUNT = 5;

	// Put this line back in if you want to see the diagnostic output
	//PduAllocator_Test() : m_pdu_alloc(m_diag) {}

protected:
	void SetUp() override
	{
		ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
		ASSERT_EQ(PDU_COUNT, m_pdu_alloc.FreePdus());
	}

	Diagnostics m_diag;
	PduAllocator<PDU_SIZE, PDU_COUNT> m_pdu_alloc;
};

TEST_F(PduAllocator_Test, CountDecrementsByOneWhenAllocating)
{
	auto pdu = m_pdu_alloc.Allocate();
	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
	ASSERT_EQ(PDU_COUNT - 1, m_pdu_alloc.FreePdus());
}

TEST_F(PduAllocator_Test, CountIncrementsByOneWhenDeallocating)
{
	{
		auto pdu = m_pdu_alloc.Allocate();
		ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
		ASSERT_EQ(PDU_COUNT - 1, m_pdu_alloc.FreePdus());
	}
	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.FreePdus());
}

TEST_F(PduAllocator_Test, CanAllocateAllPdus)
{
	std::vector<PduPtr> pdus;

	for (int i = 0; i < PDU_COUNT; ++i)
		pdus.push_back(m_pdu_alloc.Allocate());

	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
	ASSERT_EQ(0, m_pdu_alloc.FreePdus());

	pdus.clear();

	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.FreePdus());
}

TEST_F(PduAllocator_Test, CantAllocateMorePdusThanExists)
{
	std::vector<PduPtr> pdus;

	// Allocate all pdus
	for (int i = 0; i < PDU_COUNT; ++i)
	{
		auto pdu = m_pdu_alloc.Allocate();
		ASSERT_TRUE(pdu);
		pdus.push_back(std::move(pdu));
	}

	// Can't allocate after all have gone
	ASSERT_DEATH({auto pdu = m_pdu_alloc.Allocate(); }, "Assertion failed");

	pdus.clear();

	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.Capacity());
	ASSERT_EQ(PDU_COUNT, m_pdu_alloc.FreePdus());
}

TEST_F(PduAllocator_Test, DefaultAllocationIsAFullSizePdu)
{
	auto pdu = m_pdu_alloc.Allocate();
	ASSERT_EQ(PDU_SIZE, pdu->GetDataLen());
	ASSERT_EQ(0, pdu->GetOffset());
	ASSERT_EQ(PDU_SIZE, pdu->GetCapacity());
}

TEST_F(PduAllocator_Test, OffsetMustBeLessThanCapacity)
{
	ASSERT_DEATH({m_pdu_alloc.Allocate(PDU_SIZE, PDU_SIZE + 1); }, "Assertion failed");
}

TEST_F(PduAllocator_Test, DataLenMustBeLessThanCapacity)
{
	ASSERT_DEATH({ m_pdu_alloc.Allocate(PDU_SIZE + 1); }, "Assertion failed");
}

TEST_F(PduAllocator_Test, DataLenAndOffsetMustBeLessThanCapacity)
{
	ASSERT_DEATH({ m_pdu_alloc.Allocate(PDU_SIZE -2, PDU_SIZE - 2); }, "Assertion failed");
}

TEST_F(PduAllocator_Test, CustomPduDataLen)
{
	auto pdu = m_pdu_alloc.Allocate(PDU_SIZE - 2);
	ASSERT_EQ(PDU_SIZE - 2, pdu->GetDataLen());
	ASSERT_EQ(0, pdu->GetOffset());
}

TEST_F(PduAllocator_Test, CustomPduLenAndOffset)
{
	auto pdu = m_pdu_alloc.Allocate(PDU_SIZE - 2, 1);
	ASSERT_EQ(PDU_SIZE - 2, pdu->GetDataLen());
	ASSERT_EQ(1, pdu->GetOffset());
}

TEST_F(PduAllocator_Test, MemoryIsClearedAfterDeallocation)
{
	const uint8_t* pdu_mem = nullptr;
	size_t len = 0;
	{
		// WARNING: If the implementation changes, this test could break
		auto pdu = m_pdu_alloc.Allocate();
		len = pdu->GetDataLen();
		for (int i = 0; i < len; ++i)
			pdu->WriteableData()[i] = i;
		pdu_mem = pdu->Data();

		// Data should be as we set it
		for (int i = 0; i < len; ++i)
			ASSERT_EQ(i, pdu_mem[i]);
	}

	// Data should be cleared
	for (int i = 0; i < len; ++i)
		ASSERT_EQ(0, pdu_mem[i]);
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