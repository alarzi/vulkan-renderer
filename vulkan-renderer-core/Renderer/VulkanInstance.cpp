#include "VulkanInstance.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* pUserData);

VulkanInstance::VulkanInstance()
{
}

VulkanInstance::~VulkanInstance()
{
}

bool VulkanInstance::create()
{
	return create_instance();
}

void VulkanInstance::shutdown()
{
	if (VK_NULL_HANDLE != instance)
	{
		std::cout << "Destroy vulkan instance\n";
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
}

bool VulkanInstance::create_instance()
{
	std::vector<const char*> instance_extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#ifdef ENABLE_DEBUG_LAYERS
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	std::vector<const char*> debug_layers = { "VK_LAYER_LUNARG_standard_validation" };
#endif

	VkApplicationInfo application_info = {};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = APPLICATION_NAME;
	application_info.applicationVersion = 1;
	application_info.pEngineName = APPLICATION_NAME;
	application_info.engineVersion = 1;
	application_info.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &application_info;
	instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	instance_info.ppEnabledExtensionNames = instance_extensions.data();
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = nullptr;
#ifdef ENABLE_DEBUG_LAYERS
	instance_info.enabledLayerCount = static_cast<uint32_t>(debug_layers.size());
	instance_info.ppEnabledLayerNames = debug_layers.data();
#endif

	VkResult result = vkCreateInstance(&instance_info, nullptr, &this->instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cout << "cannot find a compatible Vulkan ICD\n";
		return false;
	}
	else if (result) {
		std::cout << "unknown error\n";
		return false;
	}

#ifdef ENABLE_DEBUG_LAYERS

	PFN_vkCreateDebugReportCallbackEXT _vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	// Register the debug callback
	VkDebugReportCallbackCreateInfoEXT callback_create_info = {};
	callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callback_create_info.flags = 
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callback_create_info.pfnCallback = (PFN_vkDebugReportCallbackEXT)vulkan_debug_callback;

	VkDebugReportCallbackEXT callback;
	result = _vkCreateDebugReportCallbackEXT(instance, &callback_create_info, nullptr, &callback);
#endif

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* pUserData)
{
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	std::string prefix("");

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		prefix += "ERROR:";
	};
	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		prefix += "WARNING:";
	};
	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		prefix += "PERFORMANCE:";
	};
	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		prefix += "INFO:";
	}
	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		prefix += "DEBUG:";
	}

	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << " [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		std::cerr << debugMessage.str() << "\n";
	}
	else {
		std::cout << debugMessage.str() << "\n";
	}

	fflush(stdout);

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}