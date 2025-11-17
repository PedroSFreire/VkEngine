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




class VulkanGraphicsPipeline
{
public:
	VulkanGraphicsPipeline() = default;
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
	~VulkanGraphicsPipeline();

	VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }

	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

    std::vector<char> readFile(const std::string& filename)const;

    VkShaderModule createShaderModule(const std::vector<char>& code)const;

    void createGraphicsPipeline(const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain, const VulkanRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout);


private:
	VkPipelineLayout pipelineLayout;

	VkPipeline graphicsPipeline;

	const VulkanLogicalDevice* logicalDevice;


};

