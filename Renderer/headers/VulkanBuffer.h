#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>


class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanMemoryAllocator;

struct VulkanBufferCreateInfo {
	VkBufferUsageFlags usage;
	// VMA Memory Usage use VMA_MEMORY_USAGE_AUTO wich is the default one but still specify it for clarity
	// but for safety should use VMA_MEMORY_USAGE_GPU_ONLY for performance critical buffer vertex and so on
	VmaMemoryUsage vmaUsage;
	// VMA Flags for now use VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT for most stuff and VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for staging buffers
	// include VMA_ALLOCATION_CREATE_MAPPED_BIT for permanently mapped buffers 
	VmaAllocationCreateFlags vmaFlags;
	VkDeviceSize size;
	uint32_t elementCount;
};


class VulkanBuffer
{
	public:
	VulkanBuffer() = default;
	VulkanBuffer(const VulkanBuffer&) = delete;
	~VulkanBuffer();
	VulkanBuffer(VulkanBuffer&& other) noexcept {
		buffer = other.buffer;
		allocation = other.allocation;
		allocationInfo = other.allocationInfo;
		allocatorHandle = other.allocatorHandle;
		elementCount = other.elementCount;

		other.buffer = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.allocatorHandle = nullptr;

	}

	void createBuffer( const VulkanMemoryAllocator& allocator,const  VulkanBufferCreateInfo& info);



	VkBuffer getBuffer() const { return buffer; };

	VmaAllocation getAllocation() const { return allocation; };

	VmaAllocationInfo getAllocationInfo() const { return allocationInfo; };

	uint32_t getElementCount() const { return elementCount; }



private:
	VkBuffer buffer{};

	VmaAllocation allocation{};

	VmaAllocationInfo allocationInfo{};

	const VulkanMemoryAllocator* allocatorHandle = nullptr;

	uint32_t elementCount = 0;


	
};

