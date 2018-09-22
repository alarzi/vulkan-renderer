#pragma once

#include <cassert>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanTools.h"
#include "VulkanDevice.h"

#include "../Framework/Properties.h"

struct SwapchainBuffer {
	VkImage image;
	VkImageView view;
};

class VulkanSwapchain
{

public:
	VulkanSwapchain();
	~VulkanSwapchain();
	
	operator VkSwapchainKHR() { return swapchain; };

	std::vector<SwapchainBuffer>	images;
	VkFormat						image_format;

	bool							create(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR presentation_surface, uint32_t *width, uint32_t *height);
	void							shutdown();

	bool							acquire_next_image_index(VkSemaphore semaphore, VkFence fence, uint32_t * image_index);
	VkResult						queue_present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore);

private:
	VkInstance						instance;
	VkDevice						logical_device;
	VkPhysicalDevice				physical_device;

	VkSwapchainKHR					swapchain;

	VkSurfaceKHR					presentation_surface;

	/** @brief Swapchain stuff */
	bool					create_swap_chain(VkSwapchainKHR &swapchain, uint32_t *width, uint32_t *height);
	bool					get_presentation_mode(VkSurfaceKHR presentation_surface, VkPresentModeKHR desired_present_mode, VkPresentModeKHR & present_mode);
	bool					get_presentation_surface_capabilities(VkSurfaceKHR presentation_surface, VkSurfaceCapabilitiesKHR & surface_capabilities);
	VkExtent2D				get_image_size(VkSurfaceCapabilitiesKHR &surface_capabilities);
	bool					get_presentation_surface_format(VkSurfaceKHR presentation_surface, VkSurfaceFormatKHR desired_surface_format, VkFormat & image_format, VkColorSpaceKHR & image_color_space);
	bool					get_swapchain_images(std::vector<VkImage> & swapchain_images);
	bool					create_swapchain_buffers(VkFormat image_format, std::vector<SwapchainBuffer> & swapchain_buffers);
};

