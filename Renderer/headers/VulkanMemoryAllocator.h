#pragma once

#include <vk_mem_alloc.h>

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;

class VulkanMemoryAllocator
{

	public:
	VulkanMemoryAllocator() = default;
	~VulkanMemoryAllocator() { destroyVmaAllocator(); }
	VulkanMemoryAllocator(const VulkanMemoryAllocator&) = delete;
	VulkanMemoryAllocator(VulkanMemoryAllocator&& pallocator) { allocator = pallocator.allocator;}


	void createVmaAllocator(VulkanInstance& instance, VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device);
	
	VmaAllocator getAllocator() const {	return allocator; }
	void destroyVmaAllocator() { vmaDestroyAllocator(allocator); }

private:
	VmaAllocator allocator;


};

