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

#include "Core/GUI/Theme.hpp"

namespace Lina
{
	Theme::ThemeDef			Theme::s_themeDef	 = {};
	HashMap<TypeID, size_t> Theme::s_chunkCounts = {};

	float Theme::GetBaseItemHeight(float dpiScale)
	{
		return 24.0f * 1.0f;
		// return 24.0f * dpiScale;
	}

	void Theme::SetWidgetChunkCount(TypeID tid, size_t count)
	{
		s_chunkCounts[tid] = count;
	}

	size_t Theme::GetWidgetChunkCount(TypeID tid)
	{
		size_t cc = s_chunkCounts[tid];

		if (cc == 0)
			s_chunkCounts[tid] = 150; // default;

		return s_chunkCounts[tid];
	}
} // namespace Lina
