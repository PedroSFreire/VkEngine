#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::~VulkanCommandBuffer(){
	vkDestroyCommandPool((*logicalDevice).getDevice(), commandPool, nullptr);
}

void VulkanCommandBuffer::recordCommandBuffer(uint32_t imageIndex, int frameIndex, VulkanLogicalDevice& logicalDevice, VulkanSwapChain& swapChain, VulkanGraphicsPipeline& graphicsPipeline, VulkanRenderPass& renderPass, VulkanFrameBuffers& frameBuffers) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(commandBuffers[frameIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }


    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getRenderPass();
    renderPassInfo.framebuffer = frameBuffers.getFrameBufferAtIndex(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[frameIndex], &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline.getGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain.getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[frameIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain.getSwapChainExtent();
    vkCmdSetScissor(commandBuffers[frameIndex], 0, 1, &scissor);

    vkCmdDraw(commandBuffers[frameIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[frameIndex]);

    if (vkEndCommandBuffer(commandBuffers[frameIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}



void VulkanCommandBuffer::createCommandBuffer(VulkanLogicalDevice& device ,const int MAX_FRAMES_IN_FLIGHT) {
    logicalDevice = &device;
	framesInFlight = MAX_FRAMES_IN_FLIGHT;

    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}


void VulkanCommandBuffer::createCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface) {
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
