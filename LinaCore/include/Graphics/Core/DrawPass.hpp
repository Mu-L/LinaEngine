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

#ifndef DrawPass_HPP
#define DrawPass_HPP

#include "Graphics/Data/RenderData.hpp"
#include "Data/Mutex.hpp"
#include "Data/Vector.hpp"

namespace Lina
{
	class GfxManager;
	class View;
	class IGfxResource;

	class DrawPass
	{
	public:
		DrawPass(GfxManager* gfxMan);
		virtual ~DrawPass();

		void Process(Vector<RenderableData>& drawList, const View& targetView, float drawDistance, DrawPassMask drawPassMask);
		void Draw(uint32 frameIndex, uint32 cmdListHandle);

	private:
		int32 FindInBatches(const MeshMaterialPair& pair);

	private:
		IGfxResource*		   m_objDataBuffer[FRAMES_IN_FLIGHT]  = {nullptr};
		IGfxResource*		   m_indirectBuffer[FRAMES_IN_FLIGHT] = {nullptr};
		GfxManager*			   m_gfxManager						  = nullptr;
		Mutex				   m_mtx;
		Vector<RenderableData> m_renderables;
		Vector<InstancedBatch> m_batches;
	};
} // namespace Lina

#endif
