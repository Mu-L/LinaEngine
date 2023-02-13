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

#ifndef PlatformTime_HPP
#define PlatformTime_HPP

/* PLATFORM TIME REQUIRED INTERFACE
static double GetSeconds();
static uint32 GetCycles();
static uint64 GetCycles64();
static double GetDeltaSeconds(uint32 from, uint32 to, double timeScale = 1.0);
static double GetDeltaSeconds64(uint64 from, uint64 to, double timeScale = 1.0);
*/

#ifdef LINA_PLATFORM_WINDOWS
#include "Platform/Win32/Win32PlatformTime.hpp"

namespace Lina
{
	typedef Win32PlatformTime PlatformTime;
}

#else

#endif
#endif
