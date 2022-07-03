/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef Profiler_HPP
#define Profiler_HPP

#ifdef LINA_ENABLE_PROFILING

#define MEMORY_STACK_TRACE_SIZE 20

// Headers here.
#include "Data/String.hpp"
#include "Data/IntrusiveList.hpp"
#include "Data/HashMap.hpp"
#include "Data/FixedVector.hpp"

namespace Lina
{
    struct Scope : public IntrusiveListNode
    {
    public:
        String               ThreadName = "";
        String               Name       = "";
        double               DurationNS = 0.0;
        double               StartTime  = 0.0;
        Scope*               Parent     = nullptr;
        IntrusiveList<Scope> Children;
    };

    struct Function
    {
        Function(const String& funcName, const String& threadName = "Main");
        ~Function();
    };

    struct Frame : public IntrusiveListNode
    {
        double               DurationNS = 0.0;
        double               StartTime  = 0.0;
        IntrusiveList<Scope> Scopes;
    };

    struct MemAllocationInfo
    {
        size_t         Size      = 0;
        unsigned short StackSize = 0;
        void*          Stack[MEMORY_STACK_TRACE_SIZE];
    };

    class Profiler
    {
    public:
        static Profiler* Get()
        {
            return s_instance;
        }

        void             StartFrame();
        void             StartScope(const String& scope, const String& thread = "Main");
        void             EndScope();
        void             OnAllocation(void* ptr, size_t size);
        void             OnVRAMAllocation(void* ptr, size_t size);
        void             OnFree(void* ptr);
        void             OnVRAMFree(void* ptr);
        void             DumpMemoryLeaks(const String& path);
        static Profiler* s_instance;

    private:
        void CaptureTrace(MemAllocationInfo& info);

    private:
        friend class Application;
        friend class Engine;

        void Initialize();
        void Shutdown();

        IntrusiveList<Frame>                      m_frames;
        Scope*                                    m_lastScope = nullptr;
        Frame*                                    m_lastFrame = nullptr;
        ParallelHashMap<void*, MemAllocationInfo> m_memAllocations;
        ParallelHashMap<void*, MemAllocationInfo> m_vramAllocations;
        size_t                                    m_totalMemAllocationSize  = 0;
        size_t                                    m_totalVRAMAllocationSize = 0;
        std::mutex                                m_lock;
    };

#define PROFILER_FRAME_START()               Profiler::Get()->StartFrame()
#define PROFILER_SCOPE_START(SCOPENAME, ...) Profiler::Get()->StartScope(SCOPENAME, __VA_ARGS__)
#define PROFILER_SCOPE_END                   Profiler::Get()->EndScope()
#define PROFILER_FUNC(...)                   Function func(__func__, __VA_ARGS__)
#define PROFILER_DUMPLEAKS(PATH)             Profiler::Get()->DumpMemoryLeaks(PATH)
#define PROFILER_ALLOC(PTR, SZ)              Profiler::Get()->OnAllocation(PTR, SZ)
#define PROFILER_VRAMALLOC(PTR, SZ)          Profiler::Get()->OnVRAMAllocation(PTR, SZ)
#define PROFILER_FREE(PTR)                   Profiler::Get()->OnFree(PTR)
#define PROFILER_VRAMFREE(PTR)               Profiler::Get()->OnVRAMFree(PTR)
#define PROFILER_SKIPTRACK(skip)             g_skipAllocTrack = skip
#define PROFILER_DESTROY()         \
    delete ::Profiler::s_instance; \
    ::Profiler::s_instance = nullptr
} // namespace Lina

#else

#define PROFILER_FRAME_START()
#define PROFILER_SCOPE_START(SCOPENAME, ...)
#define PROFILER_SCOPE_END
#define PROFILER_FUNC(...)
#define PROFILER_DUMPLEAKS(PATH)
#define PROFILER_ALLOC(PTR, SZ)
#define PROFILER_VRAMALLOC(PTR, SZ)
#define PROFILER_FREE(PTR)
#define PROFILER_VRAMFREE(PTR)
#define PROFILER_SKIPTRACK(skip)
#define PROFILER_DESTROY
#endif

#endif