/*
    A COBS encoder and decoder, with slight modifications
    to allow encoding of longer runs of data

    Reference:
        http://www.stuartcheshire.org/papers/COBSforToN.pdf
        https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

    Keith Fletcher
    Apr 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#pragma once

#include "IEncoder.hpp"
#include <cstdint>

class CobsEncoder : public IFrameEncoder
{
public:
    // The frame size is not limited, but the data packets
    // must be 254 bytes or less. COBS will split data into 
    // max 254 byte data 'chunks' after encoding
    static const uint32_t MAX_PACKET_SIZE = 254;

    EncodeResult Encode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len) override;
    EncodeResult Decode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len) override;
    uint32_t MaxEncodeLen(uint32_t source_len) const override;
    uint32_t MaxDecodeLen(uint32_t source_len) const override;

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