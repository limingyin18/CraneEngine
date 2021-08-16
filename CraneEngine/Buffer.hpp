#pragma once

#include <cstring>
#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace CraneVision
{
    class Buffer
    {
    public:
        /**
         * @brief Creates a buffer using VMA
         */
        explicit Buffer() = default;

        ~Buffer();

        Buffer(const Buffer& rhs) = delete;

        Buffer(Buffer&& rhs);

        Buffer& operator=(const Buffer& rhs) = delete;

        Buffer& operator=(Buffer&& rhs) = delete;

        void create(VmaAllocator alloc, VkDeviceSize size, VkBufferUsageFlags usage,
            VmaMemoryUsage vmaUsage, VmaAllocationCreateFlagBits allocationCreateFlagBits = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            const std::vector<uint32_t> pQueueFamilyIndices = std::vector<uint32_t>());

        void update(const void* srcData);

        static Buffer createStagingBuffer(const void* srcData, VkDeviceSize size,
            VmaAllocator vmaAllocator);

        template<class T>
        static void createVmaBufferFromVector(const std::vector<T>& dataVector,
            VmaAllocator vmaAllocator,
            Buffer& buffer, VkBufferUsageFlags usage,
            VmaMemoryUsage vmaUsage,
            VkQueue graphicsQueue, VkDevice device,
            VkCommandPool commandPool)
        {
            VkDeviceSize bufferSize = sizeof(T) * dataVector.size();
            Buffer stagingBuffer = Buffer::createStagingBuffer(dataVector.data(), bufferSize,
                vmaAllocator);

            buffer.create(vmaAllocator, bufferSize, usage, vmaUsage);
            copyBuffer(stagingBuffer.buffer, buffer.buffer, bufferSize,
                graphicsQueue, device, commandPool);
        };

    public:
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;
        uint32_t size;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation bufferMemory = VK_NULL_HANDLE;
    };
}