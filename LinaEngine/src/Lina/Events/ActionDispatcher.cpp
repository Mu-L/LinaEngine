/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: ActionDispatcher
Timestamp: 4/10/2019 1:26:39 PM

*/

#include "LinaPch.hpp"
#include "ActionDispatcher.hpp"  

namespace LinaEngine
{
	void* addr;
	ActionDispatcher::ActionDispatcher()
	{
		// For each action type insert a new list to the map.
		for (int i = 0; i < (ActionType::ACTION_TYPES_LASTINDEX + 1); i++)
		{
			m_ActionHandlerMap.insert(LinaMakePair(i, LinaArray<ActionHandlerBase*>()));
		}
	}

	ActionDispatcher::~ActionDispatcher()
	{
		LinaMap<uint32, LinaArray<ActionHandlerBase*>> it;

		// Clear map, no ownership action.
		m_ActionHandlerMap.erase(m_ActionHandlerMap.begin(), m_ActionHandlerMap.end());

		//m_ActionHandlers.clear();
	}

	void ActionDispatcher::DispatchAction(const ActionBase& action)
	{
		try {

			// Get the handler array corresponding to the action type.
			/*LinaArray<ActionHandlerBase*>& arr = m_ActionHandlerMap.at(action.GetActionType());

			// Iterate through the array of handlers.
			LinaArray<ActionHandlerBase*>::iterator it;
			for (it = arr.begin(); it != arr.end(); it++)
			{
				
			}*/
		}
		catch (const LinaOutOfRange& e)
		{
			//LINA_CORE_ERR("Out of Range Exception while subscribing handler at: {0} Message: {1} ", &action, e.what());
		}
	}

	void ActionDispatcher::SubscribeHandler(ActionHandlerBase * handler)
	{
		try {
			// Add the pointer to the array.
			//LinaArray<ActionHandlerBase*>& arr = m_ActionHandlerMap.at(handler->GetActionType());
			//arr.push_back(handler);
		}
		catch (const LinaOutOfRange& e)
		{
			//LINA_CORE_ERR("Out of Range Exception while subscribing handler at: {0} Message: {1} ", handler, e.what());
		}

	}

	void ActionDispatcher::UnsubscribeHandler(ActionHandlerBase * handler)
	{
		try {
			// Remove the pointer from the corresponding array.
		//	LinaArray<ActionHandlerBase*>& arr = m_ActionHandlerMap.at(handler->GetActionType());
		//	arr.erase(std::remove(arr.begin(), arr.end(), handler));
		}
		catch (const LinaOutOfRange& e)
		{
			//LINA_CORE_ERR("Out of Range Exception while subscribing handler at: {0} Message: {1} ", handler, e.what());
		}

	}
}
