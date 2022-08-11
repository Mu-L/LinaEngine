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

#ifndef GraphicsCommon_HPP
#define GraphicsCommon_HPP

#include "Math/Color.hpp"
#include "Data/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Core/SizeDefinitions.hpp"

enum VkFormat;
enum VkColorSpaceKHR;
enum VkPresentModeKHR;
enum VkCommandBufferLevel;
enum VkCommandPoolCreateFlagBits;
enum VkQueueFlagBits;
enum VkImageLayout;
enum VkPipelineBindPoint;
enum VkAttachmentLoadOp;
enum VkAttachmentStoreOp;
enum VkFenceCreateFlagBits;
enum VkCommandBufferUsageFlagBits;
enum VkShaderStageFlagBits;
enum VkPrimitiveTopology;
enum VkPolygonMode;
enum VkCullModeFlagBits;
enum VkVertexInputRate;
enum VkImageUsageFlagBits;
enum VkImageTiling;
enum VkImageAspectFlagBits;

struct VmaAllocation_T;
struct VkBuffer_T;
struct VkImage_T;

namespace Lina::Graphics
{
    enum class Format
    {
        B8G8R8A8_SRGB,
        R32G32B32_SFLOAT,
        D32_SFLOAT,
    };

    extern VkFormat GetFormat(Format f);

    enum class ColorSpace
    {
        SRGB_NONLINEAR,
    };

    extern VkColorSpaceKHR GetColorSpace(ColorSpace f);

    enum class PresentMode
    {
        Immediate,
        Mailbox,
        FIFO,
        FIFORelaxed,
    };

    extern VkPresentModeKHR GetPresentMode(PresentMode m);

    enum class CommandBufferLevel
    {
        Primary,
        Secondary,
    };

    extern VkCommandBufferLevel GetCommandBufferLevel(CommandBufferLevel lvl);

    enum class CommandPoolFlags
    {
        Transient,
        Reset,
        Protected,
    };

    extern uint32 GetCommandPoolCreateFlags(CommandPoolFlags f);

    enum class QueueFamilies
    {
        Graphics,
        Compute,
        Transfer,
        SparseBinding,
        Protected,
    };

    extern uint32 GetQueueFamilyBit(QueueFamilies f);

    enum class ImageLayout
    {
        Undefined,
        General,
        ColorOptimal,
        DepthStencilOptimal,
        DepthStencilReadOnlyOptimal,
        PresentSurface,
    };

    extern VkImageLayout GetImageLayout(ImageLayout l);

    enum class PipelineBindPoint
    {
        Graphics,
        Computer,
    };

    extern VkPipelineBindPoint GetPipelineBindPoint(PipelineBindPoint p);

    enum class LoadOp
    {
        Load,
        Clear,
        DontCare,
        None,
    };

    extern VkAttachmentLoadOp GetLoadOp(LoadOp p);

    enum class StoreOp
    {
        Store,
        DontCare,
        None,
    };

    extern VkAttachmentStoreOp GetStoreOp(StoreOp op);

    enum class FenceFlags
    {
        Signaled,
    };

    extern uint32 GetFenceFlags(FenceFlags f);

    enum class CommandBufferFlags
    {
        OneTimeSubmit,
        RenderPassContinue,
        SimultaneousUse,
    };

    extern uint32 GetCommandBufferFlags(CommandBufferFlags f);

    enum class ShaderStage
    {
        Vertex,
        TesellationControl,
        TesellationEval,
        Geometry,
        Fragment,
        Compute,
    };

    extern uint32 GetShaderStage(ShaderStage s);

    enum class Topology
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan,
        TriangleListAdjacency,
        TriangleStripAdjacency,
    };

    extern VkPrimitiveTopology GetTopology(Topology t);

    enum class PolygonMode
    {
        Fill,
        Line,
        Point,
        FillRect,
    };

    extern VkPolygonMode GetPolygonMode(PolygonMode m);

    enum class CullMode
    {
        None,
        Front,
        Back,
        FrontAndBack,
    };

    extern uint32 GetCullMode(CullMode cm);

    enum class VertexInputRate
    {
        Vertex,
        Instance
    };

    extern VkVertexInputRate GetVertexInputRate(VertexInputRate rate);

    enum class ImageUsageFlags
    {
        TransferSrc,
        TransferDest,
        Sampled,
        Storage,
        ColorAttachment,
        DepthStencilAttachment,
        TransientAttachment,
        InputAttachment,
        FragmentDensity,
    };

    extern uint32 GetImageUsage(ImageUsageFlags usage);

    enum class ImageTiling
    {
        Optimal,
        Linear
    };

    extern VkImageTiling GetImageTiling(ImageTiling tiling);

    enum ImageAspectFlags
    {
        AspectColor,
        AspectDepth,
        AspectStencil,
        AspectMetadata
    };

    extern uint32 GetImageAspectFlags(ImageAspectFlags aspectFlags);

    struct ClearValue
    {
        Color  clearColor = Color::White;
        float  depth      = 0.0f;
        uint32 stencil    = 0;
    };

    struct AttachmentReference
    {
        uint32      attachment;
        ImageLayout layout;
    };

    struct Viewport
    {
        float x        = 0.0f;
        float y        = 0.0f;
        float width    = 0.0f;
        float height   = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 0.0f;
    };

    struct AllocatedBuffer
    {
        VkBuffer_T*      buffer     = nullptr;
        VmaAllocation_T* allocation = nullptr;
    };

    struct AllocatedImage
    {
        VkImage_T*       image      = nullptr;
        VmaAllocation_T* allocation = nullptr;
    };

    struct VertexInputBinding
    {
        uint32          binding   = 0;
        uint32          stride    = 0;
        VertexInputRate inputRate = VertexInputRate::Vertex;
    };

    struct VertexInputAttribute
    {
        uint32 binding  = 0;
        uint32 location = 0;
        Format format   = Format::R32G32B32_SFLOAT;
        uint32 offset   = 0;
    };

    struct VertexInputDescription
    {
        Vector<VertexInputBinding>   bindings;
        Vector<VertexInputAttribute> attributes;
        uint32                       flags = 0;
    };

    struct PushConstantRange
    {
        uint32 offset     = 0;
        uint32 size       = 0;
        uint32 stageFlags = 0;
    };

    struct MeshPushConstants
    {
        Vector4 data         = Vector4::Zero;
        Matrix  renderMatrix = Matrix::Identity();
    };

    struct Extent3D
    {
        uint32 width  = 0;
        uint32 height = 0;
        uint32 depth  = 0;
    };

#define TO_FLAGS(X) static_cast<uint32>(X)

} // namespace Lina::Graphics

#endif
