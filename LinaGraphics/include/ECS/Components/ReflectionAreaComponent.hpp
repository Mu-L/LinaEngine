/* 
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

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

/*
Class: ReflectionArea



Timestamp: 1/23/2022 1:54:46 AM
*/

#pragma once

#ifndef ReflectionArea_HPP
#define ReflectionArea_HPP

// Headers here.
#include "ECS/Component.hpp"
#include "Math/Vector.hpp"
#include "Rendering/Texture.hpp"

namespace Lina::ECS
{
    LINA_COMPONENT("Reflection Area", "ICON_FA_MOUNTAIN", "Rendering", "true")
    struct ReflectionAreaComponent : public Component
    {
        LINA_PROPERTY("Is Local", "Bool", "If false, any object not affected by a local area will be affected by this one.")
        bool m_isLocal = false;

        LINA_PROPERTY("Is Dynamic", "Bool", "If true, the reflection data will be captured every frame for this area.")
        bool m_isDynamic = true;

        LINA_PROPERTY("Half Extents", "Vector3", "", "m_isLocal")
        Vector3 m_halfExtents = Vector3(.5f, .5f, .5f);

        LINA_PROPERTY("Resolution", "Vector2i")
        Vector2i m_resolution = Vector2i(512,512);

        LINA_PROPERTY("Draw Debug", "Bool", "Enables debug drawing for this component.")
        bool m_drawDebug = true;

        Graphics::Texture m_cubemap;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(m_isLocal, m_halfExtents, m_isDynamic, m_resolution, m_isEnabled); // serialize things by passing them to the archive
        }
    };
} // namespace Lina::ECS

#endif
