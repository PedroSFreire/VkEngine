#include "VulkanPipeline.h"
#include "../Includes/VulkanIncludes.h"

VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipeline((*logicalDevice).getDevice(), pipeline, nullptr);

    vkDestroyPipelineLayout((*logicalDevice).getDevice(), pipelineLayout, nullptr);
}


void VulkanPipeline::createPipelineLayout(const VulkanLogicalDevice& device, const VkPipelineLayoutCreateInfo& info) {

    if (vkCreatePipelineLayout(device.getDevice(), &info, nullptr,
        &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }


}

void VulkanPipeline::createPipeline(const VulkanLogicalDevice& device, const VkGraphicsPipelineCreateInfo& info) {

    if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &info,
        nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

}