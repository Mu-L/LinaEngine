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

#include "Core/Graphics/Pipeline/DescriptorSet.hpp"
#include "Common/Platform/LinaGXIncl.hpp"

namespace Lina
{
	void DescriptorSet::Create(LinaGX::Instance *lgx, const LinaGX::DescriptorSetDesc &desc)
    {
        m_lgx = lgx;
        m_handle = lgx->CreateDescriptorSet(desc);
        m_allocationCount = desc.allocationCount;
        
        for(uint32 i = 0; i < desc.allocationCount; i++)
            m_freeAllocations.push(i);
    }

    void DescriptorSet::Destroy()
    {
        m_lgx->DestroyDescriptorSet(m_handle);
    }

    void DescriptorSet::Allocate(uint32 &outIndex)
    {
        LINA_ASSERT(!m_freeAllocations.empty(), "");
        
        outIndex = m_freeAllocations.front();
        m_freeAllocations.pop();
    }

    void DescriptorSet::Free(uint32 index)
    {
        m_freeAllocations.push(index);
    }

} // namespace Lina
