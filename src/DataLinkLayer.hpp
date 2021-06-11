/*
    Datalink layer

    Keith Fletcher
    May 2021

        Datalink Frame
        --------------
        +---------------------+---------------+
        | Data (variable len) | CRC (32 bits) |
        +---------------------+---------------+

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#pragma once

#include "IProtoLayer.hpp"
#include "IEncoder.hpp"
#include "ICRCEngine.hpp"
#include "IDiagnostics.hpp"
#include "ProtoLib_Common.hpp"

template<typename TEncoder, typename TCRCEngine>
class DatalinkLayer : public IProtoLayer
{
public:
    explicit DatalinkLayer() :
        m_diagnostics(NullDiagnostics::Instance())
    { }

    DatalinkLayer(IDiagnostics& diagnostics) :
        m_diagnostics(diagnostics)
    { }

    PduPtr Encode(PduPtr pdu) override 
    { 
        if (!pdu)
        {
            m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
                << "Attempting to encode a null Pdu" << std::endl;
            assert(pdu);
            return PduPtr();
        }

        const auto len = pdu->GetDataLen();
        if (pdu->GetMaxDataLen() >= len + m_crc_engine.CrcSize())
        {
            auto crc = m_crc_engine.CalcBlock(*pdu);
            pdu->ResetCursor();
            pdu->SkipWrite(len);
            pdu->SetDataLen(len + sizeof(crc));
            pdu->PutDown(crc);
        }
        else
        {
            m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
                << "Not enough space in the Pdu to add the CRC" << std::endl;
            assert(false);
            return PduPtr();
        }

        return std::move(pdu); 
    }

    PduPtr Decode(PduPtr pdu) override 
    { 
        if (!pdu)
        {
            m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
                << "Attempting to decode a null Pdu" << std::endl;
            assert(pdu);
            return PduPtr();
        }

        // Get the CRC type
        decltype((m_crc_engine.CalcBlock)(ProtoPdu<1>())) crc;

        // Extract the CRC and resize the PDU
        const auto len = pdu->GetDataLen();
        if (len >= m_crc_engine.CrcSize())
        {
            pdu->ResetCursor();
            pdu->SkipRead(len - sizeof(crc));
            
            if (!pdu->PickUp(crc))
            {
                // Couldn't read the CRC for some reason
                m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
                    << "Could not extract the CRC from the Pdu" << std::endl;
                assert(false);
                return PduPtr();
            }
            pdu->SetDataLen(len - sizeof(crc));
        }
        else
        {
            // Too small, not enough room for a CRC
            m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Error)
                << "Not enough data in the Pdu to extract the CRC" << std::endl;
            assert(pdu->GetDataLen() >= m_crc_engine.CrcSize());
            return PduPtr();
        }

        // Calculate and compare the CRC
        pdu->ResetCursor();
        auto crc_calc = m_crc_engine.CalcBlock(*pdu);

        // CRC match?
        if (crc_calc != crc)
        {
            m_diagnostics.Log(DiagnosticDomain::Comms, DiagnosticLogLevel::Info)
                << "DataLinkLayer Decode: CRC mismatch" << std::endl;
            assert(crc_calc == crc);
            return PduPtr();
        }

        return std::move(pdu); 
    }

protected:


private:
    IDiagnostics&       m_diagnostics;
    const TEncoder      m_frame_encoder;
    TCRCEngine          m_crc_engine;
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