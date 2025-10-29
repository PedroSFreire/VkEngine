#include "VulkanFrameBuffers.h"



VulkanFrameBuffers::~VulkanFrameBuffers() {
	clean();
}





void VulkanFrameBuffers::clean() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer((*logicalDevice).getDevice(), framebuffer, nullptr);
    }
    swapChainFramebuffers.clear();
}


void VulkanFrameBuffers::createFramebuffers(VulkanLogicalDevice& device, VulkanSwapChain& swapChain, VulkanRenderPass& renderPass) {
	logicalDevice = &device;
    
    swapChainFramebuffers.resize(swapChain.getSwapChainImageViews().size());

    for (size_t i = 0; i < swapChain.getSwapChainImageViews().size(); i++) {
        VkImageView attachments[] = { swapChain.getSwapChainImageViews()[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.getRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChain.getSwapChainExtent().width;
        framebufferInfo.height = swapChain.getSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

}
