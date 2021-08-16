#include "Buffer.hpp"

using namespace std;
using namespace CraneVision;

Buffer::~Buffer()
{
    if (vmaAllocator != nullptr)
        vmaDestroyBuffer(vmaAllocator, buffer, bufferMemory);
    buffer = VK_NULL_HANDLE;
    bufferMemory = VK_NULL_HANDLE;
}

Buffer::Buffer(Buffer&& rhs)
{
    vmaAllocator = rhs.vmaAllocator;
    buffer = rhs.buffer;
    bufferMemory = rhs.bufferMemory;

    rhs.buffer = VK_NULL_HANDLE;
    rhs.bufferMemory = VK_NULL_HANDLE;
}

void Buffer::create(VmaAllocator alloc, VkDeviceSize size, VkBufferUsageFlags usage,
    VmaMemoryUsage vmaUsage, VmaAllocationCreateFlagBits allocationCreateFlagBits,
    VkSharingMode sharingMode, const vector<uint32_t> pQueueFamilyIndices)
{
    vmaAllocator = alloc;
    this->size = size;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;
    bufferInfo.queueFamilyIndexCount = pQueueFamilyIndices.size();
    bufferInfo.pQueueFamilyIndices = pQueueFamilyIndices.data();

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = vmaUsage;
    vmaAllocCreateInfo.flags = allocationCreateFlagBits;
    VmaAllocationInfo vmaAllocInfo{};
    if (vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer, &bufferMemory,
        &vmaAllocInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to create vma buffer");
}

void Buffer::update(const void* srcData)
{
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(vmaAllocator, bufferMemory, &allocInfo);
    memcpy(allocInfo.pMappedData, srcData, allocInfo.size);
}

Buffer Buffer::createStagingBuffer(const void* srcData, VkDeviceSize size,
    VmaAllocator vmaAllocator)
{
    Buffer stagingBuffer;
    stagingBuffer.create(vmaAllocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.update(srcData);

    return stagingBuffer;
}
