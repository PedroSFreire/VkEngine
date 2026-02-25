#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>

class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanImageView;


class VulkanFrameBuffers
{

public:
	VulkanFrameBuffers() = default;
	~VulkanFrameBuffers();
	VulkanFrameBuffers(const VulkanFrameBuffers&) = delete;
    VulkanFrameBuffers(VulkanFrameBuffers&& other) noexcept : 
        framebuffers(std::move(other.framebuffers)),logicalDevice(other.logicalDevice)
    {
        other.logicalDevice = nullptr;
    }


    VulkanFrameBuffers& operator=(VulkanFrameBuffers&& other) noexcept
    {
        if (this != &other)
        {
            clean(); 

            framebuffers = std::move(other.framebuffers);
            logicalDevice = other.logicalDevice;

            other.logicalDevice = nullptr;
        }
        return *this;
    }


	void clean();

	const std::vector<VkFramebuffer>& getSwapChainFramebuffers() const { return framebuffers; }

	const VkFramebuffer getFrameBufferAtIndex(size_t index) const { return framebuffers[index]; }

	void createFramebuffers(const VulkanLogicalDevice& device, const VulkanSwapChain& swapChain, const VulkanRenderPass& renderPass , const VulkanImageView& depthImageView, const VulkanImageView& colorImageView);

	void createCubeFramebuffers(const VulkanLogicalDevice& device, const VulkanRenderPass& renderPass, const std::array<VulkanImageView, 6>& cubemapFaceViews, uint32_t cubemapSize);

    void createBrdfLutFramebuffer(const VulkanLogicalDevice& device, const VulkanRenderPass& renderPass,const VulkanImageView& imageView, uint32_t texSize);
private:

	std::vector<VkFramebuffer> framebuffers;

	const VulkanLogicalDevice* logicalDevice = NULL;
};

