/*
	CRC32 (software algorithm) unit tests

	Keith Fletcher
	May 2021

	This file is Unlicensed.
	See the foot of the file, or refer to <http://unlicense.org>
*/

#include "CRC32_software.hpp"
#include "Test_Common.hpp"
#include <functional>
#include <list>
#include <vector>

using testing::ElementsAre;

class CRC32_SW_Test : public TestBase
{
protected:
	static constexpr uint32_t CUSTOM_INITIAL = 0x01020304;
	static constexpr uint32_t CUSTOM_FINAL = 0x01020304;
	static constexpr uint8_t INPUT_BUFFER[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };

	CRC32_SW m_crc;
};


TEST_F(CRC32_SW_Test, BlockCalcUnreflected_DefaultInitial)
{
	ASSERT_EQ(0x506853B6, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, false));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_DefaultInitial)
{
	ASSERT_EQ(0xA775B864, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, false));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_DefaultInitial)
{
	ASSERT_EQ(0x6DCA160A, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, true));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_DefaultInitial)
{
	ASSERT_EQ(0x261DAEE5, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, true));
}

TEST_F(CRC32_SW_Test, BlockCalcDefault_DefaultInitial)
{
	// Should default to reflected input and output
	ASSERT_EQ(0x261DAEE5, m_crc.CalcBlock(INPUT_BUFFER, sizeof(INPUT_BUFFER)));
}

TEST_F(CRC32_SW_Test, BlockCalcUnreflected_CustomInitial)
{
	ASSERT_EQ(0x74AFCC3F, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, false, CUSTOM_INITIAL));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_CustomInitial)
{
	ASSERT_EQ(0x83B227ED, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, false, CUSTOM_INITIAL));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_CustomInitial)
{
	ASSERT_EQ(0xFC33F52E, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, true, CUSTOM_INITIAL));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_CustomInitial)
{
	ASSERT_EQ(0xB7E44DC1, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, true, CUSTOM_INITIAL));
}

TEST_F(CRC32_SW_Test, BlockCalcUnreflected_CustomFinal)
{
	ASSERT_EQ(0xAE95AF4D, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, false, CRC32Calc_SW::CRC_DEFAULT_INITIAL, CUSTOM_FINAL));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_CustomFinal)
{
	ASSERT_EQ(0x5988449F, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, false, CRC32Calc_SW::CRC_DEFAULT_INITIAL, CUSTOM_FINAL));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_CustomFinal)
{
	ASSERT_EQ(0x9337EAF1, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), false, true, CRC32Calc_SW::CRC_DEFAULT_INITIAL, CUSTOM_FINAL));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_CustomFinal)
{
	ASSERT_EQ(0xD8E0521E, m_crc.CalcBlock32(INPUT_BUFFER, sizeof(INPUT_BUFFER), true, true, CRC32Calc_SW::CRC_DEFAULT_INITIAL, CUSTOM_FINAL));
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