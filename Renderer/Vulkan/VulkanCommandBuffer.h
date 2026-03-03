#pragma once


#include "../../Engine/Core/defines.h"

#include "glm/glm.hpp"

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFrameBuffers;
class VulkanPipeline;
class VulkanBuffer;
class VulkanCommandPool;
class VulkanRenderer;
class GltfLoader;
class VulkanDescriptorSet;
class Scene;
class ResourceManager;

class VulkanCommandBuffer
{
private:

	VkCommandBuffer commandBuffer;
	VkDescriptorSet currentUBO = VK_NULL_HANDLE;
	const VulkanLogicalDevice* logicalDevice = nullptr;
	const VulkanCommandPool* commandPool = nullptr;
public:
	VulkanCommandBuffer() = default;
	VulkanCommandBuffer(const VulkanLogicalDevice& device,const VulkanCommandPool& commandPool);
	~VulkanCommandBuffer();
	//VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
	
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept {
		commandBuffer = other.commandBuffer;
		currentUBO = other.currentUBO;
		logicalDevice = other.logicalDevice;
		commandPool = other.commandPool;

		other.commandBuffer = VK_NULL_HANDLE;
	}


	



	const VkCommandBuffer& getCommandBuffer() const { return commandBuffer; }


	void beginRecordindSingleTimeCommands(const VulkanLogicalDevice& device);

	void endRecordingSingleTimeCommands(const VulkanLogicalDevice& device);

	//should only be directly called if using an array of command buffers, otherwise rely on constructor and destructor to handle creation
	void createCommandBuffer(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	
	
};

