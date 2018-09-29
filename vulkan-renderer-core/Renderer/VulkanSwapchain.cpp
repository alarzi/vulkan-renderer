#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain()
{
}


VulkanSwapchain::~VulkanSwapchain()
{
}

bool VulkanSwapchain::create(VulkanInstance instance, VulkanDevice device, VulkanPresentationSurface presentation_surface, uint32_t *width, uint32_t *height)
{
	this->instance = instance;
	this->physical_device = device.physical_device;
	this->logical_device = device.logical_device;
	this->presentation_surface = presentation_surface;

	return create_swap_chain(swapchain, width, height);
}

void VulkanSwapchain::shutdown()
{
	std::cout << "Destroy swap chain images\n";
	for (uint32_t i = 0; i < images.size(); i++) {
		vkDestroyImageView(logical_device, images[i].view, nullptr);
	}

	std::cout << "Destroy swap chain\n";
	if (swapchain) {
		vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

bool VulkanSwapchain::create_swap_chain(VkSwapchainKHR& swapchain, uint32_t *width, uint32_t *height)
{
	// Get the presentation mode (triple buffer)
	VkPresentModeKHR presentation_mode;
	get_presentation_mode(presentation_surface, VK_PRESENT_MODE_MAILBOX_KHR, presentation_mode);

	// Get image format and color space
	VkColorSpaceKHR image_color_space;
	if (!presentation_surface.get_format(physical_device, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, image_format, image_color_space)) {
		return false;
	}

	// Get the gpu surface capabilities
	VkSurfaceCapabilitiesKHR surface_capabilities;
	presentation_surface.get_capabilities(physical_device, surface_capabilities);

	// Get the number of swapchain images
	uint32_t images_count = surface_capabilities.minImageCount + 1;
	if ((surface_capabilities.maxImageCount > 0) && (images_count > surface_capabilities.maxImageCount)) {
		images_count = surface_capabilities.maxImageCount;
	}

	// Check usage of swap chain image
	VkImageUsageFlags image_usage;
	image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & surface_capabilities.supportedUsageFlags;
	if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT != image_usage) {
		return false;
	}
	// Enable transfer source on swap chain images if supported
	if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagBitsKHR pre_transformation;
	if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		pre_transformation = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		pre_transformation = surface_capabilities.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < sizeof(composite_alpha_flags); i++) {
		if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flags[i]) {
			composite_alpha = composite_alpha_flags[i];
			break;
		}
	}

	VkExtent2D image_size = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surface_capabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		image_size.width = *width;
		image_size.height = *height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		image_size = surface_capabilities.currentExtent;
		*width = surface_capabilities.currentExtent.width;
		*height = surface_capabilities.currentExtent.height;
	}

	//swapchainExtent.width = std::clamp(swapchainExtent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
	//swapchainExtent.height = std::clamp(swapchainExtent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

	// Create the swapchain
	VkSwapchainKHR old_swapchain = swapchain;
	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = presentation_surface;
	swapchain_create_info.minImageCount = images_count;
	swapchain_create_info.imageFormat = image_format;
	swapchain_create_info.imageColorSpace = image_color_space;
	swapchain_create_info.imageExtent = image_size;
	swapchain_create_info.imageUsage = image_usage;
	swapchain_create_info.preTransform = pre_transformation;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = 0;
	swapchain_create_info.pQueueFamilyIndices = nullptr;
	swapchain_create_info.presentMode = presentation_mode;
	swapchain_create_info.oldSwapchain = old_swapchain;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.compositeAlpha = composite_alpha;

	VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swapchain);
	if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == swapchain)) {
		std::cout << "Could not create a swapchain.\n";
		return false;
	}

	// Destroy the old swapchain
	if (VK_NULL_HANDLE != old_swapchain) {
		vkDestroySwapchainKHR(logical_device, old_swapchain, nullptr);
		old_swapchain = VK_NULL_HANDLE;
	}

	// Create the swapchain buffers
	if (!create_swapchain_buffers(image_format, images)) {
		return false;
	}

	return true;
}

VkExtent2D VulkanSwapchain::get_image_size(VkSurfaceCapabilitiesKHR &surface_capabilities)
{
	VkExtent2D image_size = {};
	//// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	//if (surface_capabilities.currentExtent.width == (uint32_t)-1)
	//{
	//	// If the surface size is undefined, the size is set to
	//	// the size of the images requested.
	//	image_size.width = *width;
	//	image_size.height = *height;
	//}
	//else
	//{
	//	// If the surface size is defined, the swap chain size must match
	//	image_size = surface_capabilities.currentExtent;
	//	*width = surface_capabilities.currentExtent.width;
	//	*height = surface_capabilities.currentExtent.height;
	//}

	if (0xFFFFFFFF == surface_capabilities.currentExtent.width) {
		image_size = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };

		if (image_size.width < surface_capabilities.minImageExtent.width) {
			image_size.width = surface_capabilities.minImageExtent.width;
		}
		else if (image_size.width > surface_capabilities.maxImageExtent.width) {
			image_size.width = surface_capabilities.maxImageExtent.width;
		}

		if (image_size.height < surface_capabilities.minImageExtent.height) {
			image_size.height = surface_capabilities.minImageExtent.height;
		}
		else if (image_size.height > surface_capabilities.maxImageExtent.height) {
			image_size.height = surface_capabilities.maxImageExtent.height;
		}
	}
	else {
		image_size = surface_capabilities.currentExtent;
	}

	return image_size;
}

bool VulkanSwapchain::get_presentation_mode(VkSurfaceKHR presentation_surface, VkPresentModeKHR desired_present_mode, VkPresentModeKHR & present_mode)
{
	// Enumerate supported present modes
	uint32_t present_modes_count = 0;
	VkResult result = VK_SUCCESS;

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, nullptr);
	if ((VK_SUCCESS != result) || (0 == present_modes_count)) {
		std::cout << "Could not get the number of supported present modes." << std::endl;
		return false;
	}

	std::vector<VkPresentModeKHR> present_modes(present_modes_count);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, present_modes.data());
	if ((VK_SUCCESS != result) || (0 == present_modes_count)) {
		std::cout << "Could not enumerate present modes." << std::endl;
		return false;
	}

	// Select present mode
	for (auto & current_present_mode : present_modes) {
		if (current_present_mode == desired_present_mode) {
			present_mode = desired_present_mode;
			return true;
		}
	}

	std::cout << "Desired present mode is not supported. Selecting default FIFO mode." << std::endl;
	for (auto & current_present_mode : present_modes) {
		if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
			present_mode = VK_PRESENT_MODE_FIFO_KHR;
			return true;
		}
	}

	std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory for all drivers!" << std::endl;
	return false;
}

bool VulkanSwapchain::get_swapchain_images(std::vector<VkImage> & swapchain_images)
{
	uint32_t images_count = 0;
	VkResult result = VK_SUCCESS;

	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, nullptr);
	if ((VK_SUCCESS != result) || (0 == images_count)) {
		std::cout << "Could not get the number of swapchain images." << std::endl;
		return false;
	}

	swapchain_images.resize(images_count);
	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, swapchain_images.data());
	if ((VK_SUCCESS != result) || (0 == images_count)) {
		std::cout << "Could not enumerate swapchain images." << std::endl;
		return false;
	}

	return true;
}

bool VulkanSwapchain::acquire_next_image_index(VkSemaphore semaphore, VkFence fence, uint32_t *image_index)
{
	VkResult result = vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, semaphore, fence, image_index);
	if (VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result) {
		std::cout << "Could not acquire next swapchain image." << std::endl;
		return false;
	}

	return true;
}

bool VulkanSwapchain::create_swapchain_buffers(VkFormat image_format, std::vector<SwapchainBuffer> & swapchain_buffers)
{
	std::vector<VkImage> images;
	if (!get_swapchain_images(images)) {
		return false;
	}

	swapchain_buffers.resize(images.size());
	for (uint32_t i = 0; i < images.size(); i++) {
		swapchain_buffers[i].image = images[i];

		VkImageViewCreateInfo view_create_info = {};
		view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_create_info.image = swapchain_buffers[i].image;
		view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_create_info.format = image_format;
		view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
		view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
		view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
		view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_create_info.subresourceRange.baseArrayLayer = 0;
		view_create_info.subresourceRange.levelCount = 1;
		view_create_info.subresourceRange.baseArrayLayer = 0;
		view_create_info.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(logical_device, &view_create_info, nullptr, &swapchain_buffers[i].view);
		if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == swapchain)) {
			std::cout << "Could not create image view.\n";
			return false;
		}
	}
	return true;
}

VkResult VulkanSwapchain::queue_present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore = VK_NULL_HANDLE)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &image_index;
	// Check if a wait semaphore has been specified to wait for before presenting the image
	if (wait_semaphore != VK_NULL_HANDLE)
	{
		present_info.pWaitSemaphores = &wait_semaphore;
		present_info.waitSemaphoreCount = 1;
	}
	return vkQueuePresentKHR(queue, &present_info);
}