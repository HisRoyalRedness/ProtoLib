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
{};

TEST_F(CRC32_SW_Test, BlockCalcUnreflected_DefaultInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	CRC32_SW crc;
	ASSERT_EQ(0x506853B6, crc.CalcBlock32(buffer, sizeof(buffer), false, false));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_DefaultInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0xA775B864, crc.CalcBlock32(buffer, sizeof(buffer), true, false));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_DefaultInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x6DCA160A, crc.CalcBlock32(buffer, sizeof(buffer), false, true));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_DefaultInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x261DAEE5, crc.CalcBlock32(buffer, sizeof(buffer), true, true));
}

TEST_F(CRC32_SW_Test, BlockCalcDefault_DefaultInitial)
{
	// Should default to reflected input and output
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x261DAEE5, crc.CalcBlock(buffer, sizeof(buffer)));
}

TEST_F(CRC32_SW_Test, BlockCalcUnreflected_CustomInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x74AFCC3F, crc.CalcBlock32(buffer, sizeof(buffer), false, false, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_CustomInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x83B227ED, crc.CalcBlock32(buffer, sizeof(buffer), true, false, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_CustomInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0xFC33F52E, crc.CalcBlock32(buffer, sizeof(buffer), false, true, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_CustomInitial)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0xB7E44DC1, crc.CalcBlock32(buffer, sizeof(buffer), true, true, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcUnreflected_CustomFinal)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0xAE95AF4D, crc.CalcBlock32(buffer, sizeof(buffer), false, false, CRC32Calc_SW::CRC_DEFAULT_INITIAL, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcInputReflected_CustomFinal)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x5988449F, crc.CalcBlock32(buffer, sizeof(buffer), true, false, CRC32Calc_SW::CRC_DEFAULT_INITIAL, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcOutputReflected_CustomFinal)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0x9337EAF1, crc.CalcBlock32(buffer, sizeof(buffer), false, true, CRC32Calc_SW::CRC_DEFAULT_INITIAL, 0x01020304));
}

TEST_F(CRC32_SW_Test, BlockCalcInputAndOutputReflected_CustomFinal)
{
	static const uint8_t buffer[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	CRC32_SW crc;
	ASSERT_EQ(0xD8E0521E, crc.CalcBlock32(buffer, sizeof(buffer), true, true, CRC32Calc_SW::CRC_DEFAULT_INITIAL, 0x01020304));
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