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

#ifndef Lina_Color_HPP
#define Lina_Color_HPP
#include "LinaVG/Core/Vectors.hpp"

namespace Lina
{
	class Color
	{

	public:
		Color(float rv = 1.0f, float gv = 1.0f, float bv = 1.0f, float av = 1.0f, bool is255 = false) : x(is255 ? rv / 255.0f : rv), y(is255 ? gv / 255.0f : gv), z(is255 ? bv / 255.0f : bv), w(is255 ? av / 255.0f : av){};
		Color(const LinaVG::Vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w){};

		LinaVG::Vec4 AsLVG4() const
		{
			return LinaVG::Vec4(x, y, z, w);
		}

		float x, y, z, w = 1.0f;

		bool operator!=(const Color& rhs) const
		{
			return !(x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
		}

		bool operator==(const Color& rhs) const
		{
			return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
		}

		Color operator*(const float& rhs) const
		{
			return Color(x * rhs, y * rhs, z * rhs, w * rhs);
		}

		Color operator+(const Color& rhs) const
		{
			return Color(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}

		Color HSToRGB();
		Color RGBToHSV();
		Color HSVToRGB();

	public:
		static Color Red;
		static Color Green;
		static Color LightBlue;
		static Color Blue;
		static Color DarkBlue;
		static Color Cyan;
		static Color Yellow;
		static Color Black;
		static Color White;
		static Color Purple;
		static Color Maroon;
		static Color Beige;
		static Color Brown;
		static Color Gray;

		void SaveToStream(OStream& stream);
		void LoadFromStream(IStream& stream);
	};

} // namespace Lina

#endif
