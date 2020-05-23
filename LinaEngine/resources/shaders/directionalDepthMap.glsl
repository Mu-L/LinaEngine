/*
 * Copyright (C) 2019 Inan Evin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "common.glh"
#include <uniformBuffers.glh>
#include <utility.glh>


#if defined(VS_BUILD)
Layout(0) attribute vec3 position;
Layout(1) attribute vec2 texCoord;
Layout(4) attribute mat4 model;

void main()
{
    gl_Position = lightSpace * model * vec4(position, 1.0);
    //gl_Position = model * vec4(position, 1.0);
}

#elif defined(FS_BUILD)
#define BIAS 0.01

void main()
{
	// = gl_FragCoord.z;
    //gl_FragDepth += gl_FrontFacing ? BIAS : 0.0; //  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
}
#endif