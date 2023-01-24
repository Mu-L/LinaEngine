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

#ifndef Common_HPP
#define Common_HPP

// Headers here.
#include "Core/SizeDefinitions.hpp"

namespace Lina
{
    enum class PreferredGPUType
    {
        Discrete = 0,
        Integrated,
        CPU
    };

    enum class VsyncMode
    {
        None = 0,
        StrongVsync,
        Adaptive,
        TripleBuffer
    };

    enum WindowStyleOptions
    {
        WSO_Decorated = 1 << 0,
        WSO_Resizable = 1 << 1,
    };

    enum WindowInitOptions
    {
        WIO_CustomWidthHeight = 1 << 0,
        WIO_Fullscreen        = 1 << 1,
        WIO_WorkArea          = 1 << 2,
    };

    struct SystemInitializationInfo
    {
        const char*      appName            = "";
        int              windowWidth        = 0;
        int              windowHeight       = 0;
        uint16           windowStyleOptions = 0;
        uint16           windowInitOptions  = 0;
        PreferredGPUType preferredGPUType   = PreferredGPUType::Discrete;
        VsyncMode        vsyncMode          = VsyncMode::None;
    };
} // namespace Lina

#endif
