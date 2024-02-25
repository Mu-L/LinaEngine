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

#include "Common/SizeDefinitions.hpp"

namespace Lina
{

#define GUI_TEXTURE_HUE_HORIZONTAL UINT32_MAX - 1
#define GUI_TEXTURE_HUE_VERTICAL   UINT32_MAX - 2
#define GUI_TEXTURE_COLORWHEEL	   UINT32_MAX - 3


#define LINA_MAIN_SWAPCHAIN			 "LinaMainSwapchain"_hs
#define DEFAULT_SHADER_SID			 "Resources/Core/Shaders/UnlitStandard.linashader"_hs
#define DEFAULT_SHADER_GUI			 "Resources/Core/Shaders/GUIStandard.linashader"_hs
#define DEFAULT_SAMPLER_SID			 "Resources/Core/Samplers/DefaultSampler.linasampler"_hs
#define DEFAULT_GUI_SAMPLER_SID		 "Resources/Core/Samplers/DefaultGUISampler.linasampler"_hs
#define DEFAULT_GUI_TEXT_SAMPLER_SID "Resources/Core/Samplers/DefaultGUITextSampler.linasampler"_hs
#define DEFAULT_TEXTURE_LINALOGO	 "Resources/Core/Textures/StubLinaLogo.png"_hs
#define DEFAULT_TEXTURE_EMPTY_BLACK  "Resources/Core/Textures/StubBlack.png"_hs
#define DEFAULT_TEXTURE_CHECKERED	 "Resources/Core/Textures/Checkered.png"_hs
#define DEFAULT_LIT_MATERIAL		 "Resources/Core/Materials/DefaultLit.linamaterial"_hs
#define DEFAULT_UNLIT_MATERIAL		 "Resources/Core/Materials/DefaultUnlit.linamaterial"_hs
#define DEFAULT_CLEAR_CLR			 Color(0.3f, 0.3f, 0.5f, 1.0f)
#define IDEAL_RT					 16667
#define FRAMES_IN_FLIGHT			 2
#define BACK_BUFFER_COUNT			 3
#define MAX_BOUND_TEXTURES			 512
#define MAX_BOUND_SAMPLERS			 128

	enum class ShaderWriteTargetType
	{
		RenderTarget,
		Swapchain,
	};

	enum RenderPassDescriptorType
	{
		Basic = 0,
		Max,
	};

	struct SemaphoreData
	{
		uint16 semaphore;
		uint64 value;
	};

} // namespace Lina
