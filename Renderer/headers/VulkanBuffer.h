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


	void createBuffer(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice, const VkDeviceSize size, const uint32_t vertexCount, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties);

	VkBuffer getBuffer() const { return buffer; }

	VkDeviceMemory getBufferMemory()const { return bufferMemory; }

	uint32_t getVertCount() const { return vertCount; }



private:
	VkBuffer buffer{};

	VkDeviceMemory bufferMemory{};

	const VulkanLogicalDevice* logicalDevice = nullptr;

	uint32_t vertCount = 0;


	
};

