#include "VulkanCommandBuffer.h"

#include "../Includes/VulkanIncludes.h"





VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if(logicalDevice != nullptr && commandBuffer != nullptr)
        vkFreeCommandBuffers(logicalDevice->getDevice(), commandPool->getCommandPool(), 1, &commandBuffer);
}


VulkanCommandBuffer::VulkanCommandBuffer(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool)
{
	logicalDevice = &device;
	this->commandPool = &commandPool;
    createCommandBuffer(device,commandPool);
}





void VulkanCommandBuffer::createCommandBuffer(const VulkanLogicalDevice& device , const VulkanCommandPool& commandPool) {

    logicalDevice = &device;
    this->commandPool = &commandPool;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}





void VulkanCommandBuffer::beginRecordindSingleTimeCommands(const VulkanLogicalDevice& logicalDevice) {


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);


}

void VulkanCommandBuffer::endRecordingSingleTimeCommands(const VulkanLogicalDevice& logicalDevice) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(commandPool->getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(commandPool->getQueue());

}

