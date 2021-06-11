/*
    Interface for diagnostic logging

    Keith Fletcher
    June 2021

    This file is Unlicensed.
    See the foot of the file, or refer to <http://unlicense.org>
*/

#pragma once

#include "DiagnosticDomain.hpp"
#include "ProtoLib_Common.hpp"
#include <ostream>

enum class DiagnosticLogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,

    // Special values when enabling logging. Not to be used when logging messages
    None,
    All
};

//=====------------------------------------------------------------------------------
// General diagnostic logging interface
class IDiagnostics
{
public:
    virtual std::ostream& Log(DiagnosticDomain domain, DiagnosticLogLevel level) = 0;
};

//=====------------------------------------------------------------------------------
// A Null class for when we don't want to log diagnostics
class NullDiagnostics final : public IDiagnostics
{
public:
    NullDiagnostics(NullDiagnostics const&) = delete;
    void operator=(NullDiagnostics const&) = delete;

    static NullDiagnostics& Instance()
    {
        static NullDiagnostics instance;
        return instance;
    }

    std::ostream& Log(DiagnosticDomain domain, DiagnosticLogLevel level) override;

private:
    NullDiagnostics() {}

    class NullStream : public std::ostream
    {
    public:
        NullStream() : std::ostream(nullptr) {}
        NullStream(const NullStream&) : std::ostream(nullptr) {}
    } m_stream;
};

template <class T>
const NullDiagnostics::NullStream& operator<<(NullDiagnostics::NullStream&& os, const T& value)
{
    return os;
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