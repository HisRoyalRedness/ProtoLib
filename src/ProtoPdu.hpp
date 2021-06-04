/*
    Communication protocol data unit.
    Packets of data passed from one protocol layer to another

    Keith Fletcher
    May 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#pragma once

#include "ProtoLib_Common.hpp"
#include "IDiagnostics.hpp"
#include <memory>


//=====------------------------------------------------------------------------------
// An interface to the PDU without the template argument 
class IProtoPdu
{
public:
    virtual ~IProtoPdu() {};

    virtual uint8_t* Data() = 0;
    virtual size_t GetDataLen() const = 0;
    virtual void SetDataLen(size_t len) = 0;
    virtual size_t GetOffset() const = 0;
    virtual void SetOffset(size_t offset) = 0;
    virtual size_t GetCapacity() const = 0;

    virtual void ResetCursor() = 0;

    template<typename T>
    bool PutDown(T value)
    {
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            const size_t shift = (sizeof(T) - i - 1) * 8;
            T mask = (0xff << shift);
            PutDownSingle(static_cast<uint8_t>((value & mask) >> shift));
        }
        return true;
    }

protected:
    virtual bool PutDown(const uint8_t* buffer, size_t len) = 0;
    virtual bool PutDownSingle(const uint8_t data) = 0;
};




//=====------------------------------------------------------------------------------
// A statically-sized PDU
template<size_t PDU_SIZE>
class ProtoPdu final : public IProtoPdu
{
public:
    uint8_t* Data() override { return &m_data[m_offset]; }

    size_t GetOffset() const override { return m_offset; }
    void SetOffset(size_t offset) override
    { 
        assert(m_data_len + offset <= PDU_SIZE);
        m_offset = offset;
        m_cursor = offset;
    }

    size_t GetDataLen() const override { return m_data_len; }
    void SetDataLen(size_t len) override
    {
        assert(len + m_offset <= PDU_SIZE);
        m_data_len = len;
    }

    size_t GetCapacity() const override { return PDU_SIZE; }

    void Reset()
    {
        m_data_len = PDU_SIZE;
        m_offset = 0;
        std::memset(m_data, 0, sizeof(m_data));
        ResetCursor();
    }

    void ResetCursor() override
    {
        m_cursor = m_offset;
    }

    bool PutDown(const uint8_t* buffer, size_t len) override
    {
        if (m_cursor + len - m_offset > m_data_len)
        {
            // no space
            assert(false);
            return false;
        }
        else
        {
            std::memcpy(&m_data[m_cursor], buffer, len);
            m_cursor += len;
            return true;
        }
    }

    bool PutDownSingle(const uint8_t data) override
    {
        if (m_cursor + 1 - m_offset > m_data_len)
        {
            // no space
            assert(false);
            return false;
        }
        else
        {
            m_data[m_cursor++] = data;
            return true;
        }
    }

private:
    uint8_t m_data[PDU_SIZE] = { 0 };
    size_t m_data_len = PDU_SIZE;
    size_t m_offset = 0;
    size_t m_cursor = 0;
};

//=====------------------------------------------------------------------------------
// A non-templated interface for deallocation
class IPduDeallocator
{
public:
    virtual void FreePdu(IProtoPdu& pdu) = 0;
};

namespace PduAllocattion
{
    //=====--------------------------------------------------------------------------
    // The internal deallocator class
    class PduDeallocator final
    {
    public:
        PduDeallocator() : m_dealloc(nullptr) {}
        PduDeallocator(IPduDeallocator& dealloc) : m_dealloc(&dealloc) {}

        void operator()(IProtoPdu* pdu) 
        { 
            if (nullptr != m_dealloc)
                m_dealloc->FreePdu(*pdu);
            else
                assert(false);
        }

    private:
        IPduDeallocator* m_dealloc;
    };
}

typedef std::unique_ptr<IProtoPdu, PduAllocattion::PduDeallocator> PduPtr;

//=====------------------------------------------------------------------------------
// A non-templated interface to the allocator
class IPduAllocator
{
public:
    virtual PduPtr Allocate(size_t len, size_t offset) = 0;
    virtual size_t Capacity() const = 0;
    virtual size_t FreePdus() const = 0;
};


//=====------------------------------------------------------------------------------
// A container of statically-sized PDUs, with functions to allocate and return back 
// to the pool
template<size_t PDU_SIZE, size_t PDU_COUNT>
class PduAllocator final : public IPduAllocator, IPduDeallocator
{

    // TODO: Allocate and free are not thread safe. 


private:
    struct PduWrapper
    {
        ProtoPdu<PDU_SIZE>      pdu;
        bool                    is_free = true;
    };

public:
    explicit PduAllocator() :
        m_deallocator(*this),
        m_diagnostics(NullDiagnostics::Instance())
    { }

    PduAllocator(IDiagnostics& diagnostics) : 
        m_deallocator(*this),
        m_diagnostics(diagnostics)
    { }

    size_t Capacity() const override { return PDU_COUNT; }
    size_t FreePdus() const override 
    { 
        size_t free_pdus = 0;
        for (auto& wrapper : m_pdu_pool)
            if (wrapper.is_free)
                ++free_pdus;
        return free_pdus;
    }
    PduPtr Allocate(size_t len = PDU_SIZE) { return Allocate(len, 0); }
    PduPtr Allocate(size_t len, size_t offset) override
    {
        assert(len + offset <= PDU_SIZE);

        for (auto& wrapper : m_pdu_pool)
        {
            if (wrapper.is_free)
            {
                wrapper.is_free = false;
                wrapper.pdu.SetOffset(0);
                wrapper.pdu.SetDataLen(len);
                if (offset != 0)
                    wrapper.pdu.SetOffset(offset);
                m_diagnostics.Log(DiagnosticDomain::Memory, DiagnosticLogLevel::Debug) << 
                    "PDU allocated. " << FreePdus() << " of " << Capacity() << 
                    " PDUs now available." << std::endl;
                return PduPtr(dynamic_cast<IProtoPdu*>(&wrapper.pdu), m_deallocator);
            }
        }
        m_diagnostics.Log(DiagnosticDomain::Memory, DiagnosticLogLevel::Error) << "No free PDUs to allocate" << std::endl;
        assert(false);
        return PduPtr{};
    }

private:
    void FreePdu(IProtoPdu& pdu) override
    {
        for (auto& wrapper : m_pdu_pool)
        {
            if (&(wrapper.pdu) == &pdu)
            {
                assert(!wrapper.is_free); // PDU already freed
                wrapper.is_free = true;
                wrapper.pdu.Reset(); // Clear out the data
                m_diagnostics.Log(DiagnosticDomain::Memory, DiagnosticLogLevel::Debug) <<
                    "PDU freed. " << FreePdus() << " of " << Capacity() <<
                    " PDUs now available." << std::endl;
                return;
            }
        }
        m_diagnostics.Log(DiagnosticDomain::Memory, DiagnosticLogLevel::Error) << "Attempted to free an unallocated PDU" << std::endl;
        assert(false); // PDU not found
    }

    IDiagnostics&                       m_diagnostics;
    PduWrapper                          m_pdu_pool[PDU_COUNT];
    PduAllocattion::PduDeallocator      m_deallocator;
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