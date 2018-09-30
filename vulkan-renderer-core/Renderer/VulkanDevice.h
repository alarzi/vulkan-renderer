#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanTools.h"

class VulkanDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

	bool					create(VkInstance instance, VkSurfaceKHR presentation_surface);
	void					shutdown();

	VkDevice				logical_device;
	VkPhysicalDevice		physical_device;
	VkSurfaceKHR			presentation_surface;

	VkQueue					graphics_queue;
	VkQueue					compute_queue;
	VkQueue					present_queue;

	uint32_t				graphics_queue_family_index;
	uint32_t				compute_queue_family_index;
	uint32_t				present_queue_family_index;

	operator VkDevice() { return logical_device; };

	bool					get_memory_type(uint32_t type_bits, VkFlags requirement_mask, uint32_t * type_index);

private:
	/** @brief Device */
	VkInstance								instance;
		
	VkPhysicalDeviceMemoryProperties		memory_properties;

	bool									create_device();
	void create_logical_device(std::vector<const char *> &device_extensions);
	bool									get_physical_devices(std::vector<VkPhysicalDevice>& physical_devices);
	bool									check_physical_device(VkPhysicalDevice physical_device, const std::vector<const char*> device_extensions);
	bool									check_physical_device_extensions(VkPhysicalDevice physical_device, const std::vector<const char *> & desired_extensions);
	bool									get_device_extensions_properties(VkPhysicalDevice physical_device, std::vector<VkExtensionProperties>& device_extensions);
	void									get_physical_device_features_and_properties(VkPhysicalDeviceFeatures& device_features, VkPhysicalDeviceProperties& device_properties);
	void									get_physical_device_memory_properties(VkPhysicalDeviceMemoryProperties& device_memory_properties);
	bool									get_physical_device_queue_family_properties(std::vector<VkQueueFamilyProperties>& queue_family_properties);

	uint32_t								get_queue_family_index(VkQueueFlags desired_queue_flags);
	uint32_t								get_surface_queue_index(VkSurfaceKHR presentation_surface);
	std::vector<uint32_t>					get_queue_indices();
};