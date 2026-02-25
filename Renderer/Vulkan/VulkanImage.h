#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>

class VulkanLogicalDevice;
class VulkanPhysicalDevice;
class VulkanMemoryAllocator;

struct VulkanImageCreateInfo
{
	uint32_t width;
	uint32_t height;
	uint32_t layers = 1;
	VkFormat format;
	
	int mipLevels = 1;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
	// VMA Memory Usage use VMA_MEMORY_USAGE_AUTO wich is the default one but still specify it for clarity
	// but for safety should use VMA_MEMORY_USAGE_GPU_ONLY for performance critical buffer vertex and so on
	VmaMemoryUsage vmaUsage;
	// VMA Flags for now use VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT for most stuff and VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for staging buffers
	// include VMA_ALLOCATION_CREATE_MAPPED_BIT for permanently mapped buffers 
	VmaAllocationCreateFlags vmaFlags;

	VkImageCreateFlags flags = 0;
};

class VulkanImage
{

public:
	VulkanImage();
	VulkanImage(const VulkanImage&) = delete;
	~VulkanImage();
	VulkanImage(VulkanImage&& other) noexcept {
		image = other.image;
		allocationInfo = other.allocationInfo;
		allocation = other.allocation;
		allocatorHandle = other.allocatorHandle;

		other.allocatorHandle = nullptr;
		other.image = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		
	}

	void create2DImage(const VulkanMemoryAllocator& allocator,const VulkanImageCreateInfo& info);

	VkImage getImage() const { return image; }

	VkImageCreateInfo getImageInfo() const { return imageInfo; }

	VmaAllocation getAllocation() const { return allocation; };

	VmaAllocationInfo getAllocationInfo() const { return allocationInfo; };

	void clean();


private:

	VkImage image{};

	const VulkanMemoryAllocator* allocatorHandle = nullptr;

	VkImageCreateInfo imageInfo{};

	VmaAllocation allocation{};

	VmaAllocationInfo allocationInfo{};

};

