#pragma once
#include "../Includes/VulkanIncludes.h"
#include "../../Engine/Core/defines.h"
#include "VulkanRenderer.h"
#include "../../Engine/Resources/ResourceManager.h"



class CommandBufferRecorder {

public:
	CommandBufferRecorder() = default;
	CommandBufferRecorder(const CommandBufferRecorder&) = delete;
	~CommandBufferRecorder();


	static void bindMesh(VulkanCommandBuffer& commandBuffer, const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer);

	static void recordDrawCall(VulkanRenderer& renderer,VulkanCommandBuffer& commandBuffer, const VulkanPipeline& graphicsPipeline, const CPUDrawCallData data,  ResourceManager& resourceManager, VkDescriptorSet currentUBO);

	static void recordCommandBufferForwardPass(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, SceneFramesData& drawData, VkDescriptorSet descriptorSet, ResourceManager& resourceManager);

	static void recordCommandBufferCopyBuffer(VulkanCommandBuffer& commandBuffer, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize size);

	static void recordCommandBufferCubePass(const VulkanRenderer& renderer, VulkanRenderPass& renderpass, VulkanFrameBuffers& frameBuffers, VulkanCommandBuffer& commandBuffer,
													VulkanPipeline& pipeline, uint32_t cubemapSize, VulkanDescriptorSet& texDescriptor, const MeshBuffers& cube);

	static void recordCommandBufferBrdfLutPass(const VulkanRenderer& renderer, VulkanRenderPass& renderpass, VulkanFrameBuffers& frameBuffers, VulkanCommandBuffer& commandBuffer,
		VulkanPipeline& pipeline, uint32_t texSize, VulkanDescriptorSet& texDescriptor);

	static void recordCommandBufferPreFilteredCubePass(const VulkanRenderer& renderer, VulkanRenderPass& renderpass, VulkanFrameBuffers& frameBuffers, VulkanCommandBuffer& commandBuffer,
		VulkanPipeline& pipeline, uint32_t cubemapSize, VulkanDescriptorSet& texDescriptor, const MeshBuffers& cube, float roughness);


};