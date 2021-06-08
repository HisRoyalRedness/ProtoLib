/*
    Calculate a CRC32 checksum, without a hardware CRC engine

    Keith Fletcher
    May 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#pragma once

#include "ICRCEngine.hpp"
#include "ProtoLib_Common.hpp"

// Helper method to switch the bit endianness of a given value
template<typename T>
constexpr T Reflect(T val)
{
    constexpr T BITS = sizeof(T) * 8;
    T resVal = 0;

    for (int i = 0; i < BITS; i++)
    {
        if ((val & (1 << i)) != 0)
        {
            resVal |= (1 << (BITS - 1 - i));
        }
    }

    return resVal;
}


// Performs a single stateful per-byte CRC calculation
class CRC32Calc_SW : public ICRCCalculation<uint32_t>
{
public:
    static constexpr uint32_t CRC_DEFAULT_INITIAL   = 0xFFFFFFFF;
    static constexpr uint32_t CRC_DEFAULT_FINAL     = 0xFFFFFFFF;
    static constexpr uint32_t CRC_POLYNOMIAL        = 0x04C11DB7;
    static constexpr bool DEFAULT_REFLECT_INPUT     = true;
    static constexpr bool DEFAULT_REFLECT_OUTPUT    = true;

    CRC32Calc_SW(
        bool reflect_input = DEFAULT_REFLECT_INPUT,
        bool reflect_output = DEFAULT_REFLECT_OUTPUT,
        uint32_t initial = CRC_DEFAULT_INITIAL,
        uint32_t final = CRC_DEFAULT_FINAL) :
        m_initial(initial),
        m_final(final),
        m_reflect_input(reflect_input), 
        m_reflect_output(reflect_output),
        m_accumulator(initial)
    {}

    void Reset() override;
    uint32_t Complete() override;

    void AddData(uint8_t data) override;

private:
    const uint32_t m_initial                = CRC_DEFAULT_INITIAL;
    const uint32_t m_final                  = CRC_DEFAULT_FINAL;
    uint32_t m_accumulator                  = CRC_DEFAULT_INITIAL;
    bool m_reflect_input                    = DEFAULT_REFLECT_INPUT;
    bool m_reflect_output                   = DEFAULT_REFLECT_OUTPUT;
};

// Performs a stateless block CRC calculation
class CRC32_SW: public ICRCEngine<uint32_t>
{
public:
    // ICRCEngine
    uint32_t CalcBlock(PduPtr pdu) override;
    uint32_t CalcBlock(const uint8_t* buffer, size_t buffer_len) override;


    uint32_t CalcBlock32(
        const uint8_t* buffer, 
        size_t buffer_len, 
        bool reflect_input = CRC32Calc_SW::DEFAULT_REFLECT_INPUT,
        bool reflect_output = CRC32Calc_SW::DEFAULT_REFLECT_OUTPUT,
        uint32_t initial = CRC32Calc_SW::CRC_DEFAULT_INITIAL,
        uint32_t final = CRC32Calc_SW::CRC_DEFAULT_FINAL);
};


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