#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include "Buffer.hpp"

namespace Crane
{
    class Image
    {
    public:
        explicit Image() = default;
        ~Image();

        Image(Image &&rhs);
        Image(const Image &rhs) = delete;
        Image &operator=(const Image &rhs) = delete;
        Image &operator=(Image &&rhs);

        void create(VmaAllocator alloc, uint32_t width, uint32_t height, uint32_t channels,
                    VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VmaMemoryUsage vmaUsage);
        void update(const void *srcData, vk::CommandBuffer cmdBuff);

        void updateBuffer(VkBuffer buffer, vk::CommandBuffer cmdBuff);

        void copyBufferToImage(VkBuffer buffer, VkCommandBuffer commandBuffer);
        void transitionImageLayout(VkFormat format,
                                   VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkCommandBuffer commandBuffer);

    public:
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation imageMemory = VK_NULL_HANDLE;
        VkExtent3D extent;
        VkDeviceSize imageSize;
    };
}