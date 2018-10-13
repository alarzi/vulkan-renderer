#pragma once

#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "VulkanTools.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanPresentationSurface.h"

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

	uint32_t						width;
	uint32_t						height;

	bool							create(VulkanInstance instance, VulkanDevice device, VulkanPresentationSurface presentation_surface, uint32_t* width, uint32_t* height);
	void							shutdown();

	bool							acquire_next_image_index(VkSemaphore semaphore, VkFence fence, uint32_t* image_index);
	VkResult						queue_present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore);

private:
	VkInstance						instance;
	VkDevice						logical_device;
	VkPhysicalDevice				physical_device;

	VkSwapchainKHR					swapchain;

	VulkanPresentationSurface		presentation_surface;

	/** @brief Swapchain stuff */
	bool					create_swapchain();
	bool					create_swapchain(const VkSwapchainKHR &old_swapchain);
	bool					get_presentation_mode(VkSurfaceKHR presentation_surface, VkPresentModeKHR desired_present_mode, VkPresentModeKHR &present_mode);
	bool					get_swapchain_images(std::vector<VkImage> &swapchain_images);
	bool					create_swapchain_buffers(VkFormat image_format, std::vector<SwapchainBuffer> &swapchain_buffers);
};

