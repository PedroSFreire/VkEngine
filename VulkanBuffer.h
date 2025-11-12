#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class VulkanPhysicalDevice;
class VulkanLogicalDevice;



class VulkanBuffer
{
	public:
	VulkanBuffer() = default;
	VulkanBuffer(const VulkanBuffer&) = delete;
	~VulkanBuffer();
	VulkanBuffer(VulkanBuffer&& other) noexcept {
		buffer = other.buffer;
		bufferMemory = other.bufferMemory;
		logicalDevice = other.logicalDevice;
		vertCount = other.vertCount;

		other.buffer = VK_NULL_HANDLE;
		other.bufferMemory = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;

	}


	void createBuffer(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& logicalDevice, VkDeviceSize size,uint32_t vertexCount, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	//void createVertexBuffer(VulkanPhysicalDevice& physicalDevice , VulkanLogicalDevice& logicalDevice);
	VkBuffer& getBuffer() { return buffer; }
	VkDeviceMemory& getBufferMemory() { return bufferMemory; }
	uint32_t getVertCount() const { return vertCount; }


private:
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	VulkanLogicalDevice* logicalDevice;
	uint32_t vertCount = 0;

	
};

