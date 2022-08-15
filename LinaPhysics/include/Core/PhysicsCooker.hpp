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

#ifndef PhysXCooker_HPP
#define PhysXCooker_HPP

// Headers here.
#include "Core/CommonApplication.hpp"
#include "Core/SizeDefinitions.hpp"
#include "Math/Vector.hpp"
#include "Data/Vector.hpp"

namespace Lina::Event
{
    struct EShutdown;
    struct EResourceLoaded;
} // namespace Lina::Event

namespace physx
{
    class PxFoundation;
}

namespace Lina::Physics
{
    class PhysXCooker
    {

    public:
        PhysXCooker() = default;
        ~PhysXCooker() = default;
        void Initialize(physx::PxFoundation* foundation);
        void Shutdown();

    private:
        /// <summary>
        /// Uses the cooking library to create a convex mesh stream using the given vertices.
        /// Stream is placed into the given buffer data for custom serialization.
        /// </summary>
        void CookConvexMesh(Vector<Vector3>& vertices, Vector<uint8>& bufferData);

        void OnResourceLoadCompleted(const Event::EResourceLoaded& ev);
        // void CookModelNodeVertices(Graphics::ModelNode& node, Graphics::Model& model);

    private:
    };
} // namespace Lina::Physics

#endif
