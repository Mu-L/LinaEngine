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

#include "Graphics/Core/View.hpp"

namespace Lina
{
	bool View::IsVisible(const AABB& aabb) const
	{
		return true;
		FrustumTest test = m_frustum.TestIntersection(aabb);
		return test != FrustumTest::Outside;
	}

	void View::Tick(const Vector3& pos, const Matrix4& view, const Matrix4& proj, float near, float far)
	{
		m_pos  = pos;
		m_view = view;
		m_proj = proj;
		m_near = near;
		m_far  = far;
		m_frustum.Calculate(proj * view, false);
	}

	void View::FillGPUViewData(GPUViewData& vd)
	{
		vd.camNearFar = Vector2(GetNear(), GetFar());
		vd.camPos	  = GetPos();
		vd.proj		  = GetProj();
		vd.view		  = GetView();
		vd.viewProj	  = vd.proj * vd.view;
	}
} // namespace Lina
