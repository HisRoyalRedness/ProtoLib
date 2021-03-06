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

    virtual const uint8_t* Data() const = 0;
    virtual size_t GetDataLen() const = 0;
    virtual size_t GetMaxDataLen() const = 0;
    virtual bool SetDataLen(size_t len) = 0;
    virtual size_t GetOffset() const = 0;
    virtual bool SetOffset(size_t offset) = 0;
    virtual size_t GetCapacity() const = 0;

#ifdef UNITTEST
    // Access points created specifically to make unit testing a little easier
    virtual uint8_t* WriteableData() = 0;
    virtual size_t GetReadCursor() const = 0;
    virtual size_t GetWriteCursor() const = 0;
#endif

    virtual void ResetCursor() = 0;

    virtual bool SkipRead(size_t bytes_to_skip) = 0;
    virtual bool SkipWrite(size_t bytes_to_skip) = 0;

    // The general case, for types that we haven't specialised
    template<typename T>
    bool PutDown(T value)
    {
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            const size_t shift = (sizeof(T) - i - 1) * 8;
            T mask = (static_cast<T>(0xff) << shift);
            if (!PutDownSingle(static_cast<uint8_t>((value & mask) >> shift)))
                return false;
        }
        return true;
    }

    template<typename T>
    bool PutDownRev(T value)
    {
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            if (!PutDownSingleRev(static_cast<uint8_t>(value & 0xff)))
                return false;
            value >>= 8;
        }
        return true;
    }

    // PutDown - specialised cases
    template<> bool PutDown(int8_t value) { return PutDownSingle(static_cast<uint8_t>(value)); }
    template<> bool PutDown(uint8_t value) { return PutDownSingle(value); }
    template<> bool PutDown(int16_t value) { return PutDownSingle(static_cast<uint16_t>(value)); }
    template<> bool PutDown(uint16_t value) { return
            PutDownSingle(static_cast<uint8_t>((value & 0xff00) >> 8)) &&
            PutDownSingle(static_cast<uint8_t>(value & 0xff)); }
    template<> bool PutDown(int32_t value) { return PutDownSingle(static_cast<uint32_t>(value)); }
    template<> bool PutDown(uint32_t value) { return 
            PutDownSingle(static_cast<uint8_t>((value & 0xff000000) >> 24)) && 
            PutDownSingle(static_cast<uint8_t>((value & 0xff0000) >> 16)) &&
            PutDownSingle(static_cast<uint8_t>((value & 0xff00) >> 8)) &&
            PutDownSingle(static_cast<uint8_t>(value & 0xff)); }
#ifndef UNITTEST
    // Excluded so unit tests can test the general case
    template<> bool PutDown(int64_t value) { return PutDownSingle(static_cast<uint64_t>(value)); }
#endif
    template<> bool PutDown(uint64_t value) { return
        PutDownSingle(static_cast<uint8_t>((value & 0xff00000000000000) >> 56)) && 
        PutDownSingle(static_cast<uint8_t>((value & 0xff000000000000) >> 48)) &&
        PutDownSingle(static_cast<uint8_t>((value & 0xff0000000000) >> 40)) &&
        PutDownSingle(static_cast<uint8_t>((value & 0xff00000000) >> 32)) &&
        PutDownSingle(static_cast<uint8_t>((value & 0xff000000) >> 24)) &&
        PutDownSingle(static_cast<uint8_t>((value & 0xff0000) >> 16)) &&
        PutDownSingle(static_cast<uint8_t>((value & 0xff00) >> 8)) &&
        PutDownSingle(static_cast<uint8_t>(value & 0xff)); }

    template<> bool PutDownRev(int8_t value) { return PutDownSingleRev(static_cast<uint8_t>(value)); }
    template<> bool PutDownRev(uint8_t value) { return PutDownSingleRev(value); }
    template<> bool PutDownRev(int16_t value) { return PutDownSingleRev(static_cast<uint16_t>(value)); }
    template<> bool PutDownRev(uint16_t value) { return
            PutDownSingleRev(static_cast<uint8_t>(value)) && 
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)); }
    template<> bool PutDownRev(int32_t value) { return PutDownSingleRev(static_cast<uint32_t>(value)); }
    template<> bool PutDownRev(uint32_t value) { return 
            PutDownSingleRev(static_cast<uint8_t>(value)) && 
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)); }
#ifndef UNITTEST
    // Excluded so unit tests can test the general case
    template<> bool PutDownRev(int64_t value) { return PutDownSingleRev(static_cast<uint64_t>(value)); }
#endif
    template<> bool PutDownRev(uint64_t value) { return
            PutDownSingleRev(static_cast<uint8_t>(value)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)) &&
            PutDownSingleRev(static_cast<uint8_t>(value >>= 8)); }

    bool PutDown(const uint8_t* value, size_t len)
    {
        for (size_t i = 0; i < len; ++i)
            if (!PutDownSingle(value[i]))
                return false;
        return true;
    }

    bool PutDownRev(const uint8_t* value, size_t len)
    {
        for (size_t i = len; i > 0; ++i)
            if (!PutDownSingleRev(value[i - 1]))
                return false;
        return true;
    }

    template<typename T>
    bool PickUp(T& value)
    {
        value = 0;
        uint8_t byte = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            if (PickUpSingle(byte))
                value = (value << 8) | byte;
            else
            {
                value = 0;
                return false;
            }
        }
        return true;
    }

    template<typename T>
    bool PickUpRev(T& value)
    {
        value = 0;
        uint8_t byte = 0;
        int shift = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            if (PickUpSingleRev(byte))
            {
                value += static_cast<T>(byte) << shift;
                shift += 8;
            }
            else
            {
                value = 0;
                return false;
            }
        }
        return true;
    }

protected:
    /// <summary>
    /// Put down a single byte and increment the cursor by 1
    /// </summary>
    /// <param name="data">The byte to write at the current cursor location</param>
    /// <returns>True is the write was successful, false otherwise</returns>
    virtual bool PutDownSingle(const uint8_t data) = 0;
    /// <summary>
    /// Put down a single byte and decrement the cursor by 1
    /// </summary>
    /// <param name="data">The byte to write at the current cursor location</param>
    /// <returns>True is the write was successful, false otherwise</returns>
    virtual bool PutDownSingleRev(const uint8_t data) = 0;
    virtual bool PickUpSingle(uint8_t& data) = 0;
    virtual bool PickUpSingleRev(uint8_t& data) = 0;
};


//=====------------------------------------------------------------------------------
// A statically-sized PDU
template<size_t PDU_SIZE>
class ProtoPdu final : public IProtoPdu
{
public:
    const uint8_t* Data() const override { return &m_data[m_offset]; }

    size_t GetOffset() const override { return m_offset; }
    bool SetOffset(size_t offset) override
    { 
        if (m_data_len + offset > PDU_SIZE)
        {
            assert(false);
            return false;
        }

        m_offset = offset;
        if (m_read_cursor < offset)
            m_read_cursor = offset;
        if (m_write_cursor < offset)
            m_write_cursor = offset;
        return true;
    }

    size_t GetDataLen() const override { return m_data_len; }
    bool SetDataLen(size_t len) override
    {
        if (m_data_len + m_offset > PDU_SIZE)
        {
            assert(false);
            return false;
        }
        m_data_len = len;
        return true;
    }

    size_t GetMaxDataLen() const override { return PDU_SIZE - m_offset; }

#ifdef UNITTEST
    uint8_t* WriteableData() override { return &m_data[m_offset]; }
    size_t GetReadCursor() const { return m_read_cursor - m_offset; }
    size_t GetWriteCursor() const { return m_write_cursor - m_offset; }
#endif

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
        m_read_cursor = m_offset;
        m_write_cursor = m_offset;
    }

    bool SkipRead(size_t bytes_to_skip) override
    {
        if (m_read_cursor + bytes_to_skip - m_offset > m_data_len)
        {
            // no space
            assert(m_read_cursor + bytes_to_skip - m_offset <= m_data_len);
            return false;
        }
        else
        {
            m_read_cursor += bytes_to_skip;
            return true;
        }
    }

    bool SkipWrite(size_t bytes_to_skip) override
    {
        if (m_write_cursor + bytes_to_skip - m_offset > m_data_len)
        {
            // no space
            assert(m_write_cursor + bytes_to_skip - m_offset <= m_data_len);
            return false;
        }
        else
        {
            m_write_cursor += bytes_to_skip;
            return true;
        }
    }

    /// <summary>
    /// Put down a single byte and increment the cursor by 1
    /// </summary>
    /// <param name="data">The byte to write at the current cursor location</param>
    /// <returns>True is the write was successful, false otherwise</returns>
    bool PutDownSingle(const uint8_t data) override
    {
        if (m_write_cursor + 1 - m_offset > m_data_len)
        {
            // no space
            assert(m_write_cursor + 1 - m_offset <= m_data_len);
            return false;
        }
        else
        {
            m_data[m_write_cursor++] = data;
            return true;
        }
    }

    /// <summary>
    /// Put down a single byte and decrement the cursor by 1
    /// </summary>
    /// <param name="data">The byte to write at the current cursor location</param>
    /// <returns>True is the write was successful, false otherwise</returns>
    bool PutDownSingleRev(const uint8_t data) override
    {
        if (m_write_cursor <= m_offset)
        {
            // no space
            assert(m_write_cursor > m_offset);
            return false;
        }
        else
        {
            m_data[--m_write_cursor] = data;
            return true;
        }
    }

    bool PickUpSingle(uint8_t& data) override
    {
        if (m_read_cursor - m_offset >= m_data_len)
        {
            // No more data to pick
            assert(m_read_cursor - m_offset < m_data_len);
            return false;
        }
        else
        {
            data = m_data[m_read_cursor++];
            return true;
        }
    }

    bool PickUpSingleRev(uint8_t& data) override
    {
        if (m_read_cursor <= m_offset)
        {
            // No more data to pick
            assert(m_read_cursor > m_offset);
            return false;
        }
        else
        {
            data = m_data[--m_read_cursor];
            return true;
        }
    }

private:
    uint8_t m_data[PDU_SIZE] = { 0 };
    size_t m_data_len = PDU_SIZE;
    size_t m_offset = 0;
    size_t m_read_cursor = 0;
    size_t m_write_cursor = 0;
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
                assert(nullptr == m_dealloc);
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