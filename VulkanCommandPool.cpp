#include "VulkanCommandPool.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSurface.h"


VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(logicalDevice->getDevice(), commandPool, nullptr);
}








void VulkanCommandPool::createGraphicsCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface) {
    
    logicalDevice = &device;
	queue = &device.getGraphicsQueue();
    QueueFamilyIndices queueFamilyIndices = physicalDevice.findQueueFamilies(surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanCommandPool::createTransferCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface) {
    
	logicalDevice = &device;
	queue = &device.getTransferQueue();

    QueueFamilyIndices queueFamilyIndices = physicalDevice.findQueueFamilies(surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();


    if (vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}


