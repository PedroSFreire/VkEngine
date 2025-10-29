#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"


class VulkanGraphicsPipeline
{
private:
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

	VulkanLogicalDevice* logicalDevice;
public:
	VulkanGraphicsPipeline() = default;
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
	~VulkanGraphicsPipeline();

	VkPipeline& getGraphicsPipeline() { return graphicsPipeline; }

    std::vector<char> readFile(const std::string& filename);


    VkShaderModule createShaderModule(VulkanLogicalDevice& logicalDevice, const std::vector<char>& code);

    void createGraphicsPipeline(VulkanLogicalDevice& logicalDevice, VulkanSwapChain& swapChain, VulkanRenderPass& renderPass);


};

