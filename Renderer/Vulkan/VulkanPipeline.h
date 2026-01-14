#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <array>

class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;


struct Vertex;




class VulkanPipeline
{
public:
	VulkanPipeline() = default;
	VulkanPipeline(const VulkanPipeline&) = delete;
	~VulkanPipeline();

	VkPipeline getPipeline() const { return pipeline; }

	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

	void setLogicalDevice(const VulkanLogicalDevice& device) { logicalDevice = &device; }

	void createPipelineLayout(const VulkanLogicalDevice& device,const VkPipelineLayoutCreateInfo& info);

	void createPipeline(const VulkanLogicalDevice& device,const VkGraphicsPipelineCreateInfo& info);

private:
	VkPipelineLayout pipelineLayout;

	VkPipeline pipeline;

	const VulkanLogicalDevice* logicalDevice;


};

