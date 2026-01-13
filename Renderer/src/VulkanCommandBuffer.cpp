#include "..\headers\VulkanRenderer.h"
#include "..\headers\VulkanCommandBuffer.h"
#include "..\headers\VulkanPhysicalDevice.h"
#include "..\headers\VulkanLogicalDevice.h"
#include "..\headers\VulkanSwapChain.h"
#include "..\headers\VulkanRenderPass.h"
#include "..\headers\VulkanFrameBuffers.h"
#include "..\headers\VulkanPipeline.h"
#include "..\headers\VulkanBuffer.h"
#include "..\headers\VulkanCommandPool.h"
#include "..\headers\VulkanDescriptorSet.h"

#include "..\headers\GltfLoader.h"
#include "..\headers\Scene.h"
#include "..\headers\ResourceManager.h"












void VulkanCommandBuffer::createCommandBuffer(const VulkanLogicalDevice& device , const VulkanCommandPool& commandPool) {


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





void VulkanCommandBuffer::beginRecordindSingleTimeCommands(const VulkanLogicalDevice& logicalDevice, const VulkanCommandPool& commandPool) {


    createCommandBuffer(logicalDevice, commandPool);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);


}

void VulkanCommandBuffer::endRecordingSingleTimeCommands(const VulkanLogicalDevice& logicalDevice, const VulkanCommandPool& commandPool) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(commandPool.getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(commandPool.getQueue());
}

