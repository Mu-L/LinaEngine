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

#include "Core/Graphics/Pipeline/Buffer.hpp"
#include "Core/Graphics/Pipeline/RenderPass.hpp"

namespace Lina
{
	class Shader;
	class Entity;
	class WorldRenderer;
	class EntityWorld;
	class ResourceManagerV2;
	class Material;
	class DrawCollector;
	class Texture;
} // namespace Lina

namespace LinaGX
{
	class Instance;
}

namespace Lina::Editor
{
	class Editor;

	class OutlineSelectionRenderer
	{
	private:
		struct PerFrameData
		{
			Texture* depthTarget  = nullptr;
			Texture* renderTarget = nullptr;
		};

	public:
		OutlineSelectionRenderer(Editor* editor, WorldRenderer* wr);
		virtual ~OutlineSelectionRenderer();

		void Initialize();
		void Shutdown();
		void DestroySizeRelativeResources();
		void CreateSizeRelativeResources();
		void AddBuffersToUploadQueue(uint32 frameIndex, ResourceUploadQueue& queue);
		void Tick(float delta, DrawCollector& collector);
		void Render(uint32 frameIndex, LinaGX::CommandStream* stream, DrawCollector& collector);
		void RenderFullscreen(DrawCollector& collector, LinaGX::CommandStream* stream);
		void SyncRender();

		inline void SetSelectedEntities(const Vector<Entity*>& entities)
		{
			m_selectedEntities = entities;
		}

		inline Texture* GetRenderTarget(uint32 frameIndex) const
		{
			return m_pfd[frameIndex].renderTarget;
		}

	private:
		LinaGX::Instance*  m_lgx	= nullptr;
		Editor*			   m_editor = nullptr;
		WorldRenderer*	   m_wr		= nullptr;
		ResourceManagerV2* m_rm		= nullptr;
		EntityWorld*	   m_world	= nullptr;

		Vector<Entity*> m_selectedEntities	 = {};
		Material*		m_fullscreenMaterial = nullptr;
		Shader*			m_fullscreenShader	 = nullptr;

		uint16	   m_pipelineLayout = 0;
		RenderPass m_outlinePass;
		Vector2ui  m_size = Vector2ui::Zero;

		PerFrameData m_pfd[FRAMES_IN_FLIGHT];
	};
} // namespace Lina::Editor
