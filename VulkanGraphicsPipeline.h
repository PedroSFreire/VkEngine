#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"

#include <glm/glm.hpp>
#include <array>


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
		return attributeDescriptions;
	}

};




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

	VkPipelineLayout& getPipelineLayout() { return pipelineLayout; }

    std::vector<char> readFile(const std::string& filename);


    VkShaderModule createShaderModule(VulkanLogicalDevice& logicalDevice, const std::vector<char>& code);

    void createGraphicsPipeline(VulkanLogicalDevice& logicalDevice, VulkanSwapChain& swapChain, VulkanRenderPass& renderPass, VkDescriptorSetLayout& descriptorSetLayout);


};

