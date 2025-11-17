#include "VulkanCommandBuffer.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffers.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"


void VulkanCommandBuffer::recordCommandBuffer(const uint32_t imageIndex, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain, const VulkanGraphicsPipeline& graphicsPipeline, const VulkanRenderPass& renderPass, const VulkanFrameBuffers& frameBuffers, const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer, const VkDescriptorSet& descriptorSet) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }


    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getRenderPass();
    renderPassInfo.framebuffer = frameBuffers.getFrameBufferAtIndex(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    


    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline.getGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain.getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain.getSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getGraphicsPipeline());

    VkBuffer vertexBuffers[] = { vertBuffer.getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, indexBuffer.getVertCount(), 1, 0, 0, 0);


    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}



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


void VulkanCommandBuffer::recordCommandBufferCopyBuffer(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VkBuffer&  srcBuffer, const VkBuffer&   dstBuffer, const VkDeviceSize size) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);
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