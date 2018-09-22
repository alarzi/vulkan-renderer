#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#define VK_ERROR_STRING( x ) case static_cast< int >( x ): return #x

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << vks::tools::error_string(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
	}																									\
}

namespace vks
{
	namespace tools
	{
		/** @brief Returns an error code as a string */
		std::string error_string(VkResult errorCode);

		bool is_extension_supported(std::vector<VkExtensionProperties> const& available_extensions, char const* const extension);
		bool get_supported_depth_format(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);
	}
}
