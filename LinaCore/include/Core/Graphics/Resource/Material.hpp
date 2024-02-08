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

#ifndef Material_HPP
#define Material_HPP

#include "Core/Resources/Resource.hpp"
#include "Core/Graphics/Data/RenderData.hpp"
#include "Core/Graphics/Pipeline/Buffer.hpp"

namespace LinaGX
{
	class Instance;
}

namespace Lina
{
	class MaterialPropertyBase;
	class Shader;
	class GfxManager;

	class Material : public Resource
	{
	public:
		struct BindingData
		{
			Vector<Buffer>	 buffers;
			Vector<StringID> textures;
			Vector<StringID> samplers;

			void SaveToStream(OStream& stream) const;
			void LoadFromStream(LinaGX::Instance* lgx, IStream& stream);
		};

		Material(ResourceManager* rm, bool isUserManaged, const String& path, StringID sid);
		virtual ~Material();

		void SetShader(StringID sid);

	protected:
		virtual void LoadFromFile(const char* path) override;
		virtual void SaveToStream(OStream& stream) const override;
		virtual void LoadFromStream(IStream& stream) override;
		virtual void BatchLoaded() override;

	private:
		void DestroyBindingData();

	private:
		LinaGX::Instance*	m_lgx				   = nullptr;
		GfxManager*			m_gfxManager		   = nullptr;
		Shader*				m_shader			   = nullptr;
		StringID			m_shaderSID			   = 0;
		uint16				m_descriptorSet		   = 0;
		bool				m_descriptorSetCreated = false;
		Vector<BindingData> m_bindingData;
	};

} // namespace Lina

#endif
