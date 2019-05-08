/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions 
and limitations under the License.

Class: CameraSystem
Timestamp: 5/2/2019 12:40:46 AM

*/

#pragma once

#ifndef CameraSystem_HPP
#define CameraSystem_HPP

#include "ECS/EntityComponentSystem.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "Rendering/GameRenderContext.hpp"

using namespace LinaEngine::Graphics;

namespace LinaEngine::ECS
{
	class CameraSystem : public BaseECSSystem
	{
	public:

		CameraSystem() : BaseECSSystem()
		{
			AddComponentType(TransformComponent::ID);
			AddComponentType(CameraComponent::ID);
		}

		FORCEINLINE void Construct(GameRenderContext& contextIn)
		{
			context = &contextIn;
		}

		virtual void UpdateComponents(float delta, BaseECSComponent** components);
		


		FORCEINLINE Matrix& GetSkyboxViewTransformation()
		{
			return m_SkyboxViewTransformation;
		}

		FORCEINLINE void SetProjectionMatrix(const Matrix& projection)
		{
			m_Projection = projection;
		}

	private:

		Matrix m_Projection;
		Matrix m_SkyboxViewTransformation;
		GameRenderContext* context;
	};
}


#endif