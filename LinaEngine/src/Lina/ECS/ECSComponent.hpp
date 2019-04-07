/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions 
and limitations under the License.

Class: ECSComponent
Timestamp: 4/7/2019 3:24:08 PM

*/

#pragma once

#ifndef ECSComponent_HPP
#define ECSComponent_HPP


namespace LinaEngine
{

	/* We don't have to know about the entity, we just keep a void pointer. */
	typedef void* EntityHandle;

	/* Base strcut for ECS components. */
	struct BaseECSComponent
	{

		/* Increments id */
		static uint32 nextID();

		/* Entity reference */
		EntityHandle entityID;

	};

	template<typename T>
	struct ECSComponent : public BaseECSComponent
	{

	};

	struct TestComponent : public ECSComponent<TestComponent>
	{
		
	};
}


#endif