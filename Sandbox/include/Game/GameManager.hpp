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

#ifndef GameManager_HPP
#define GameManager_HPP

namespace Lina
{
    namespace Event
    {
        struct EStartGame;
        struct ETick;
        struct ELevelInstalled;
        struct EResourceLoaded;
    } // namespace Event
} // namespace Lina

class GameManager
{
public:
    GameManager()  = default;
    ~GameManager() = default;

    void Initialize();

private:
    void OnGameStarted(const Lina::Event::EStartGame& ev);
    void OnTick(const Lina::Event::ETick& ev);
    void OnLevelInstalled(const Lina::Event::ELevelInstalled& ev);
};
#endif

#include "Core/Component.hpp"

namespace Game
{
    LINA_COMPONENT("My Test Comp", "Default")
    struct MyTestComponent : public Lina::World::Component
    {
        virtual Lina::TypeID GetTID() override
        {
            return Lina::GetTypeID<MyTestComponent>();
        }

        virtual Lina::Bitmask16 GetComponentMask() override
        {
            return Lina::World::ComponentMask::ReceiveOnTick | Lina::World::ComponentMask::ReceiveOnPostPhysicsTick;
        }
    };
} // namespace Game
