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
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	// VMA Memory Usage use VMA_MEMORY_USAGE_AUTO wich is the default one but still specify it for clarity
	// but for safety should use VMA_MEMORY_USAGE_GPU_ONLY for performance critical buffer vertex and so on
	VmaMemoryUsage vmaUsage;
	// VMA Flags for now use VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT for most stuff and VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for staging buffers
	// include VMA_ALLOCATION_CREATE_MAPPED_BIT for permanently mapped buffers 
	VmaAllocationCreateFlags vmaFlags; 
};

class VulkanImage
{

public:
	VulkanImage() = default;
	VulkanImage(const VulkanImage&) = delete;
	~VulkanImage();
	VulkanImage(VulkanImage&& other) noexcept {
		image = other.image;
		allocationInfo = other.allocationInfo;
		allocationInfo = other.allocationInfo;
		allocatorHandle = other.allocatorHandle;


		other.image = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.allocatorHandle = nullptr;
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

