#include "VulkanDevice.h"


VulkanDevice::VulkanDevice()
{
}


VulkanDevice::~VulkanDevice()
{
}

bool VulkanDevice::create(VkInstance instance, VkSurfaceKHR presentation_surface)
{
	this->instance = instance;
	this->presentation_surface = presentation_surface;
	return this->create_device();
}

void VulkanDevice::shutdown()
{
	if (VK_NULL_HANDLE != logical_device)
	{
		std::cout << "Destroy logical device\n";
		vkDeviceWaitIdle(logical_device);
		vkDestroyDevice(logical_device, nullptr);
		logical_device = nullptr;
	}
}

bool VulkanDevice::create_device()
{
	std::vector<VkPhysicalDevice> physical_devices;
	get_physical_devices(physical_devices);

	std::vector<const char*> device_extensions;
	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	for (auto& physical_device : physical_devices)
	{
		this->physical_device = physical_device;

		// Get the features and properties of the physical device
		VkPhysicalDeviceFeatures device_features;
		VkPhysicalDeviceProperties device_properties;
		get_physical_device_features_and_properties(device_features, device_properties);

		// Get the memory properties of the physical device
		get_physical_device_memory_properties(memory_properties);

		// If the physical device doesn't support geometry shader,
		// skip to the next device
		if (!device_features.geometryShader) {
			continue;
		}

		if (!check_physical_device_extensions( device_extensions)) {
			continue;
		}

		// Get the queue family indices
		graphics_queue_family_index = get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
		compute_queue_family_index = get_queue_family_index(VK_QUEUE_COMPUTE_BIT);
		present_queue_family_index = get_surface_queue_index(presentation_surface);
		if (graphics_queue_family_index == UINT32_MAX ||
			present_queue_family_index == UINT32_MAX) {
			std::cout << "Could not find queues for graphics and presentation.\n";
			continue;
		}		

		std::vector<uint32_t> queues_indices = { graphics_queue_family_index };
		if (graphics_queue_family_index != compute_queue_family_index) {
			queues_indices.push_back(compute_queue_family_index);
		}
		if ((graphics_queue_family_index != present_queue_family_index) &&
			(compute_queue_family_index != present_queue_family_index)) {
			queues_indices.push_back(present_queue_family_index);
		}

		// Create the queues creation informations and logical device
		const float default_queue_priority(0.0f);
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		for (auto& queue_index : queues_indices) {
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queue_index;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &default_queue_priority;

			queue_create_infos.push_back(queueInfo);
		}

		// Create the logical device
		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());;
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.pEnabledFeatures = &device_features;
		device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		device_create_info.ppEnabledExtensionNames = device_extensions.data();

		VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device);
		if ((result != VK_SUCCESS) || (logical_device == VK_NULL_HANDLE)) {
			std::cout << "Could not create the logical device." << std::endl;
			continue;
		}

		// Get the graphic and compute queue from the device
		vkGetDeviceQueue(logical_device, graphics_queue_family_index, 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, compute_queue_family_index, 0, &compute_queue);
		vkGetDeviceQueue(logical_device, present_queue_family_index, 0, &present_queue);

		return true;
	}
	return false;
}


bool VulkanDevice::get_physical_devices(std::vector<VkPhysicalDevice>& physical_devices)
{
	uint32_t devices_count = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &devices_count, nullptr);
	if (result != VK_SUCCESS || devices_count == 0)
	{
		std::cout << "Could not get the number of available physical devices." << std::endl;
		return false;
	}
	physical_devices.resize(devices_count);
	result = vkEnumeratePhysicalDevices(instance, &devices_count, physical_devices.data());
	if (result != VK_SUCCESS || devices_count == 0)
	{
		std::cout << "Could not enumerate physical devices." << std::endl;
		return false;
	}

	return true;
}

bool VulkanDevice::check_physical_device_extensions(const std::vector<const char *> & desired_extensions)
{
	// Get the physical device extensions
	std::vector<VkExtensionProperties> device_extensions_properties;
	get_device_extensions_properties(device_extensions_properties);

	// Check that the desired extensions are supported by the physical device
	for (auto & extension : desired_extensions) {
		if (!vks::tools::is_extension_supported(device_extensions_properties, extension)) {
			std::cout << "Extension named '" << extension << "' is not supported by a physical device." << std::endl;
			return false;
		}
	}

	return true;
}

bool VulkanDevice::get_device_extensions_properties(std::vector<VkExtensionProperties>& device_extensions)
{
	uint32_t extensions_count = 0;
	VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
	if (result != VK_SUCCESS || extensions_count == 0)
	{
		std::cout << "Could not get the number of devices extension properties.." << std::endl;
		return false;
	}
	device_extensions.resize(extensions_count);
	result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, device_extensions.data());
	if (result != VK_SUCCESS || extensions_count == 0)
	{
		std::cout << "Could not enumerate devices extension properties." << std::endl;
		return false;
	}
	return true;
}

void VulkanDevice::get_physical_device_features_and_properties(VkPhysicalDeviceFeatures& device_features, VkPhysicalDeviceProperties& device_properties)
{
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);
}

void VulkanDevice::get_physical_device_memory_properties(VkPhysicalDeviceMemoryProperties& device_memory_properties)
{
	vkGetPhysicalDeviceMemoryProperties(physical_device, &device_memory_properties);
}

bool VulkanDevice::get_memory_type(uint32_t type_bits, VkFlags requirements_mask, uint32_t * type_index)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
		if ((type_bits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*type_index = i;
				return true;
			}
		}
		type_bits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

bool VulkanDevice::get_physical_device_queue_family_properties(std::vector<VkQueueFamilyProperties>& queue_family_properties)
{
	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
	if (queue_families_count == 0) {
		std::cout << "Could not get the number of queue families." << std::endl;
		return false;
	}
	queue_family_properties.resize(queue_families_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_family_properties.data());
	if (queue_families_count == 0) {
		std::cout << "Could not get the number of queue families." << std::endl;
		return false;
	}

	return true;
}

uint32_t VulkanDevice::get_surface_queue_index(VkSurfaceKHR presentation_surface)
{
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	if (!get_physical_device_queue_family_properties(queue_family_properties)) {
		return UINT32_MAX;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i) {
		VkBool32 presentation_supported = VK_FALSE;
		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, presentation_surface, &presentation_supported);
		if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
			return i;
		}
	}

	return UINT32_MAX;
}

uint32_t VulkanDevice::get_queue_family_index(VkQueueFlags desired_queue_flags)
{
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	if (!get_physical_device_queue_family_properties(queue_family_properties)) {
		return UINT32_MAX;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i) {
		if ((queue_family_properties[i].queueCount > 0) &&
			((queue_family_properties[i].queueFlags & desired_queue_flags) == desired_queue_flags)) {
			return i;
		}
	}

	return UINT32_MAX;
}
