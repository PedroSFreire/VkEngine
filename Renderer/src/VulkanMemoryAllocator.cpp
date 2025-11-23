#include "..\headers\VulkanMemoryAllocator.h"
#include "..\headers\VulkanInstance.h"
#include "..\headers\VulkanPhysicalDevice.h"
#include "..\headers\VulkanLogicalDevice.h"


	void VulkanMemoryAllocator::createVmaAllocator(VulkanInstance& instance, VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device) {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice.getPhysicalDevice();
		allocatorInfo.device = device.getDevice();
		allocatorInfo.instance = instance.getInstance();
		if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
			throw std::runtime_error("failed to create VMA allocator!");
		}
	}

