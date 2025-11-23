#include "..\headers\VulkanBuffer.h"

#include "..\headers\VulkanPhysicalDevice.h"
#include "..\headers\VulkanLogicalDevice.h"
#include "..\headers\VulkanMemoryAllocator.h"


VulkanBuffer::~VulkanBuffer() {
        vmaDestroyBuffer(allocatorHandle->getAllocator(), buffer, allocation);
}




void VulkanBuffer::createBuffer( const VulkanMemoryAllocator& allocator,const VulkanBufferCreateInfo& info) {
    allocatorHandle = &allocator;

    elementCount = info.elementCount;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = info.size;
    bufferInfo.usage = info.usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = info.vmaUsage;
	allocInfo.flags = info.vmaFlags;


    if (vmaCreateBuffer(allocator.getAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

}




