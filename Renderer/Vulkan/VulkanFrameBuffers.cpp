#include "VulkanFrameBuffers.h"
#include "../Includes/VulkanIncludes.h"


VulkanFrameBuffers::~VulkanFrameBuffers() {
	clean();
}





void VulkanFrameBuffers::clean() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer((*logicalDevice).getDevice(), framebuffer, nullptr);
    }
    swapChainFramebuffers.clear();
}


void VulkanFrameBuffers::createFramebuffers(const VulkanLogicalDevice& device, const VulkanSwapChain& swapChain, const VulkanRenderPass& renderPass , const VulkanImageView& depthImageView) {
	logicalDevice = &device;
    
    swapChainFramebuffers.resize(swapChain.getSwapChainImageViews().size());

    for (size_t i = 0; i < swapChain.getSwapChainImageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = { swapChain.getSwapChainImageViews()[i], depthImageView.getImageView()};
        
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.getRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain.getSwapChainExtent().width;
        framebufferInfo.height = swapChain.getSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

}
