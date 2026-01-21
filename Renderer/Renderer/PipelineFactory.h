#pragma once
#include "../Includes/VulkanIncludes.h"
#include "../../Engine/Core/defines.h"
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <array>


#include <memory>

class PipelineFactory
{
	public:

		PipelineFactory() = default;
		PipelineFactory(const PipelineFactory&) = delete;
		~PipelineFactory();

		static std::vector<char> readFile(const std::string& filename);

		static VkShaderModule createShaderModule(const VulkanLogicalDevice& device, const std::vector<char>& code);

		static void createGraphicsPipeline(VulkanPipeline& pipeline, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain, const VulkanRenderPass& renderPass, const VkDescriptorSetLayout* descriptorSetLayout, VkSampleCountFlagBits msaaSamples);

};
