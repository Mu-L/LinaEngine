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

#ifndef D3D12SurfaceRenderer_HPP
#define D3D12SurfaceRenderer_HPP

#include "D3D12Renderer.hpp"

#include "Graphics/Platform/D3D12/WinHeaders/d3d12.h"
#include "Core/StringID.hpp"
#include <wrl/client.h>
#include <dxgi1_6.h>

namespace Lina
{

	class D3D12SurfaceRenderer : public D3D12Renderer
	{
	public:
		D3D12SurfaceRenderer(D3D12GfxManager* gfxManager, StringID sid, void* windowHandle, const Vector2i& initialSize, Bitmask16 mask);
		virtual ~D3D12SurfaceRenderer();

		inline StringID GetSID()
		{
			return m_sid;
		}

	private:
		Bitmask16									 m_mask			= 0;
		void*										 m_windowHandle = nullptr;
		Vector2i									 m_size			= Vector2i::Zero;
		StringID									 m_sid			= 0;
		Microsoft::WRL::ComPtr<IDXGISwapChain3>		 m_swapchain;
	};
} // namespace Lina

#endif