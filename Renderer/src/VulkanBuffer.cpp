#include "..\headers\VulkanBuffer.h"

#include "..\headers\VulkanPhysicalDevice.h"
#include "..\headers\VulkanLogicalDevice.h"

VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(logicalDevice->getDevice(), buffer, nullptr);
        vkFreeMemory(logicalDevice->getDevice(), bufferMemory, nullptr);
  
}







void VulkanBuffer::createBuffer(const VulkanPhysicalDevice& physicalDevice , const VulkanLogicalDevice& device , const VkDeviceSize size, const uint32_t vertexCount, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties) {
    logicalDevice = &device;
	vertCount = vertexCount;



    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = physicalDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
}


