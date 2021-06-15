/*
    A DLE encoder and decoder

    Keith Fletcher
    Apr 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#include "DleEncoder.hpp"

static const EncodeResult EMPTY = { 0, 0 };

PduPtr DleEncoder::Encode(PduPtr pdu)
{
    if (!pdu)
    {
        m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
            << "Attempting to encode a null Pdu" << std::endl;
        assert(pdu);
        return PduPtr();
    }

    uint32_t max_len = MaxEncodeLen(pdu->GetDataLen());
    if (pdu->GetMaxDataLen() < max_len)
    {
        m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
            << "Not enough capacity in the Pdu to encode the data. Requires " << 
            max_len << " bytes, " << pdu->GetCapacity() << " bytes available." <<
            std::endl;
        assert(false);
        return PduPtr();
    }

    // Worst case is double the size. So, if we start reading from the end of the data,
    // and start writing at the end of the capacity of the PDU, the write should never
    // overtake the read. It means we migth finish in the middle of the buffer, but we 
    // can just set the offset to account for that
    pdu->ResetCursor();
    pdu->SkipRead(pdu->GetDataLen());
    pdu->SetDataLen(pdu->GetMaxDataLen());
    pdu->SkipWrite(pdu->GetDataLen());
    
    uint8_t data = 0;
    while (pdu->PickUpRev(data))
    {
        if (STX == data || DLE == data || ETX == data)
        {
            pdu->PutDownRev(data ^ DLE);
            pdu->PutDownRev(DLE);
        }
        else
            pdu->PutDownRev(data);
    }

    return std::move(pdu);
}

//EncodeResult DleEncoder::Encode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len)
//{
//    if (source_len == 0)
//        return EMPTY;
//
//    uint32_t max_len = MaxEncodeLen(source_len);
//    assert(target_len >= max_len);
//    if (target_len < max_len)
//        return EMPTY;
//
//    assert(source);
//    assert(target);
//
//    const uint8_t* source_cur = source;
//    uint8_t* target_cur = target;
//
//    // Target len should always be greater than 1. We need
//    // to write 2 bytes if the data must be escaped
//    while (source_cur < source + source_len && target_cur < target + target_len - 1)
//    {
//        // Write the escape byte, escape the data, and write the escaped data
//        if (*source_cur == STX || *source_cur == ETX || *source_cur == DLE)
//        {
//            *target_cur++ = DLE;
//            *target_cur++ = *source_cur++ ^ DLE;
//        }
//        else
//        {
//            *target_cur++ = *source_cur++;
//        }
//    }
//
//    return EncodeResult(
//        static_cast<uint32_t>(source_cur - source), 
//        static_cast<uint32_t>(target_cur - target));
//}

EncodeResult DleEncoder::Decode(const uint8_t* source, uint32_t source_len, uint8_t* target, uint32_t target_len)
{
    if (source_len == 0)
        return EMPTY;

    uint32_t max_len = MaxDecodeLen(source_len);
    assert(target_len >= max_len);
    if (target_len < max_len)
        return EMPTY;

    assert(source);
    assert(target);

    const uint8_t* source_cur = source;
    uint8_t* target_cur = target;

    while (source_cur < source + source_len && target_cur < target + target_len)
    {
        switch (*source_cur)
        {

        // Data is escaped.
        case DLE:
        {
            // Is there another char after the escape?
            // If not, jump out, as we can't unescape 
            // until we receive some more data
            if (source_cur >= source + source_len - 1)
                return EncodeResult(
                    static_cast<uint32_t>(source_cur - source),
                    static_cast<uint32_t>(target_cur - target),
                    true);
            else
            {
                // Consume the DLE
                source_cur++;
                // Is DLE followed by another reserved character? It shouldn't be...
                if (STX == *source_cur || ETX == *source_cur || DLE == *source_cur)
                {
                    --source_cur; // Rewind the DLE
                    return EncodeResult(
                        static_cast<uint32_t>(source_cur - source),
                        static_cast<uint32_t>(target_cur - target),
                        true);
                }
                *target_cur++ = *source_cur++ ^ DLE; // Consume and unescape the data
            }
            break;
        }

        // Invalid characters during decoding. Return an error
        case STX:
        case ETX:
            return EncodeResult(
                static_cast<uint32_t>(source_cur - source),
                static_cast<uint32_t>(target_cur - target),
                true);

        // A valid, non-escaped character
        default:
            *target_cur++ = *source_cur++;
            break;
        }
    }

    return EncodeResult(
        static_cast<uint32_t>(source_cur - source),
        static_cast<uint32_t>(target_cur - target));
}

// Calculate the worst-case size needed to encode a message
uint32_t DleEncoder::MaxEncodeLen(uint32_t source_len) const
{
    return source_len * 2;
}

// Calculate the worst-case size needed to decode a message
uint32_t DleEncoder::MaxDecodeLen(uint32_t source_len) const
{
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