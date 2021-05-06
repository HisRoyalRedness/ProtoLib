/*
    A COBS encoder and decoder
    http://www.stuartcheshire.org/papers/COBSforToN.pdf

    Keith Fletcher
    Apr 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#include "CobsEncoder.hpp"
#include <cstdint>
#include <cassert>

static const uint8_t FRAME_MARKER = 0;
static const EncodeResult EMPTY = { 0, 0 };


EncodeResult CobsEncoder::Encode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len)
{
    if (source_len == 0)
        return EMPTY;

    uint32_t max_len = MaxEncodeLen(source_len);
    assert(target_len >= max_len);
    if (target_len < max_len)
        return EMPTY;

    assert(source);
    assert(target);

    uint8_t* encode = target; // Encoded byte pointer
    uint8_t* codep = encode++; // Output code pointer
    uint8_t code = 1; // Code value
    const uint8_t* byte = source;

    for (; source_len--; ++byte)
    {
        if (*byte != FRAME_MARKER) // Byte not zero, write it
            *encode++ = *byte, ++code;

        if (*byte == FRAME_MARKER || code == 0xff) // Input is zero or block completed, restart
        {
            *codep = code, code = 1, codep = encode;
            if (*byte == FRAME_MARKER || source_len)
                ++encode;
        }
    }
    *codep = code; // Write final code value

    return EncodeResult(byte - source, encode - target);
} 

EncodeResult CobsEncoder::Decode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len)
{
    if (source_len == 0)
        return EMPTY;

    uint32_t max_len = MaxDecodeLen(source_len);
    assert(target_len >= max_len);
    if (target_len < max_len)
        return EMPTY;

    assert(source);
    assert(target);

    const uint8_t* byte = source; // Encoded input byte pointer
    uint8_t* decode = target; // Decoded output byte pointer
    uint8_t block = 0;

    for (uint8_t code = 0xff; byte < source + source_len; --block)
    {
        // Finding a frame marker in data to decode is an error
        if (FRAME_MARKER == *byte)
            return EncodeResult(byte - source, decode - target, true);

        if (block) // Decode block byte
            *decode++ = *byte++;
        else
        {
            if (code != 0xff) // Encoded zero, write it
                *decode++ = FRAME_MARKER;
            block = code = *byte++; // Next block length

            // If len is greater than the number of bytes left to read,
            // we have an error
            if (block > (source_len - (byte - source) + 1))
                return EncodeResult(byte - source, decode - target, true);
        }
    }

    return EncodeResult(byte - source, decode - target, block != 0);
}

uint32_t CobsEncoder::MaxEncodeLen(uint32_t source_len) const
{
    return source_len + ((source_len - 1) / 254) + 2;
}

uint32_t CobsEncoder::MaxDecodeLen(uint32_t source_len) const
{
    // Target should never be larger than source when decoding
    return source_len; 
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