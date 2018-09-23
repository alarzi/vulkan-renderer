#include "VulkanPresentationSurface.h"

VulkanPresentationSurface::VulkanPresentationSurface()
{
}

VulkanPresentationSurface::~VulkanPresentationSurface()
{
}

bool VulkanPresentationSurface::create(VkInstance instance, HINSTANCE hInstance, HWND hWnd)
{
	this->instance = instance;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hinstance = hInstance;
	surface_create_info.hwnd = hWnd;

	VkResult result = vkCreateWin32SurfaceKHR(instance, &surface_create_info, nullptr, &presentation_surface);
	if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == presentation_surface)) {
		std::cout << "Could not create presentation surface." << std::endl;
		return false;
	}
	return true;
}

void VulkanPresentationSurface::shutdown()
{
	if (presentation_surface) {
		std::cout << "Destroy presentation surface\n";
		vkDestroySurfaceKHR(instance, presentation_surface, nullptr);
		presentation_surface = VK_NULL_HANDLE;
	}
}

bool VulkanPresentationSurface::get_format(
	VkPhysicalDevice physical_device,
	VkSurfaceFormatKHR desired_surface_format, 
	VkFormat& format, 
	VkColorSpaceKHR& color_space)
{
	
	uint32_t formats_count = 0;
	VkResult result = VK_SUCCESS;

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, nullptr);
	if ((VK_SUCCESS != result) ||
		(0 == formats_count)) {
		std::cout << "Could not get the number of supported surface formats." << std::endl;
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, surface_formats.data());
	if ((VK_SUCCESS != result) || (0 == formats_count)) {
		std::cout << "Could not enumerate supported surface formats." << std::endl;
		return false;
	}

	// Select surface format
	if ((1 == surface_formats.size()) && (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
		format = desired_surface_format.format;
		color_space = desired_surface_format.colorSpace;
		return true;
	}

	for (auto & surface_format : surface_formats) {
		if ((desired_surface_format.format == surface_format.format) &&
			(desired_surface_format.colorSpace == surface_format.colorSpace)) {
			format = desired_surface_format.format;
			color_space = desired_surface_format.colorSpace;
			return true;
		}
	}

	for (auto & surface_format : surface_formats) {
		if ((desired_surface_format.format == surface_format.format)) {
			surface_format.format = desired_surface_format.format;
			surface_format.colorSpace = surface_format.colorSpace;
			std::cout << "Desired combination of format and colorspace is not supported. Selecting other colorspace." << std::endl;
			return true;
		}
	}

	format = surface_formats[0].format;
	color_space = surface_formats[0].colorSpace;
	std::cout << "Desired format is not supported. Selecting available format - colorspace combination." << std::endl;
	return true;
}

bool VulkanPresentationSurface::get_capabilities(VkPhysicalDevice physical_device, VkSurfaceCapabilitiesKHR& surface_capabilities)
{
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, presentation_surface, &surface_capabilities);
	if (VK_SUCCESS != result) {
		std::cout << "Could not get the capabilities of a presentation surface." << std::endl;
		return false;
	}
	return true;
}