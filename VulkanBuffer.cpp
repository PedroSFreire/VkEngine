#include "VulkanBuffer.h"



VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(logicalDevice->getDevice(), buffer, nullptr);
        vkFreeMemory(logicalDevice->getDevice(), bufferMemory, nullptr);
  
}

uint32_t VulkanBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,VulkanPhysicalDevice& physicalDevice) {
    uint32_t foundType = 0;
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            if(typeFilter == i) return i;
            foundType = i;
        }
    }
    if(foundType != 0)
		return foundType;
    throw std::runtime_error("failed to find suitable memory type!");

}


void VulkanBuffer::createBuffer(VulkanPhysicalDevice& physicalDevice , VulkanLogicalDevice& device , VkDeviceSize size,uint32_t vertexCount, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
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
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
}


