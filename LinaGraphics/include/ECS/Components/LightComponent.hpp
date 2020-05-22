/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions 
and limitations under the License.

Class: LightComponent
Timestamp: 5/13/2019 9:00:55 PM

*/

#pragma once

#ifndef LightComponent_HPP
#define LightComponent_HPP

#include "Utility/Math/Color.hpp"
#include "Utility/Math/Vector.hpp"
#include "Utility/Math/Math.hpp"


namespace LinaEngine::ECS
{

	struct LightComponent
	{
		Color color;
	};

	struct PointLightComponent : public LightComponent
	{
		float distance;
	};

	struct SpotLightComponent : public LightComponent
	{
		Vector3 direction = Vector3::Zero;
		float distance;
		float cutOff = Math::Cos(Math::ToRadians(12.5f));
		float outerCutOff = Math::Cos(Math::ToRadians(17.5f));
	};

	struct DirectionalLightComponent : public LightComponent
	{
		
	};
}


#endif