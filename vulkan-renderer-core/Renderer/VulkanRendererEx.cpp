#include "VulkanRenderer.h"



bool VulkanRenderer::reset_command_buffer(
	VkCommandBuffer command_buffer,
	bool release_resources)
{
	VkResult result = vkResetCommandBuffer(command_buffer, release_resources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
	if (VK_SUCCESS != result) {
		std::cout << "Error occurred during command buffer reset." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::reset_command_pool(
	VkDevice logical_device,
	VkCommandPool command_pool,
	bool release_resources)
{
	VkResult result = vkResetCommandPool(logical_device, command_pool, release_resources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
	if (VK_SUCCESS != result) {
		std::cout << "Error occurred during command pool reset." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::create_semaphore(
	VkDevice logical_device,
	VkSemaphore & semaphore)
{
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.flags = 0;

	VkResult result = vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &semaphore);
	if (VK_SUCCESS != result) {
		std::cout << "Could not create a semaphore." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::create_fence(
	VkDevice logical_device,
	bool signaled,
	VkFence  & fence)
{
	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u;

	VkResult result = vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);
	if (VK_SUCCESS != result) {
		std::cout << "Could not create a fence." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::wait_for_fences(
	VkDevice logical_device,
	std::vector<VkFence> const & fences,
	VkBool32 wait_for_all,
	uint64_t timeout)
{
	if (fences.size() > 0) {
		VkResult result = vkWaitForFences(logical_device, static_cast<uint32_t>(fences.size()), fences.data(), wait_for_all, timeout);
		if (VK_SUCCESS != result) {
			std::cout << "Waiting on fence failed." << std::endl;
			return false;
		}
		return true;
	}
	return false;
}

bool VulkanRenderer::reset_fences(
	VkDevice logical_device,
	std::vector<VkFence> const & fences)
{
	if (fences.size() > 0) {
		VkResult result = vkResetFences(logical_device, static_cast<uint32_t>(fences.size()), fences.data());
		if (VK_SUCCESS != result) {
			std::cout << "Error occurred when tried to reset fences." << std::endl;
			return false;
		}
		return VK_SUCCESS == result;
	}
	return false;
}
/*
bool VulkanRenderer::submit_command_buffers_to_queue(
	VkQueue queue,
	std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
	std::vector<VkCommandBuffer> command_buffers,
	std::vector<VkSemaphore> signal_semaphores,
	VkFence fence)
{
	std::vector<VkSemaphore>          wait_semaphore_handles;
	std::vector<VkPipelineStageFlags> wait_semaphore_stages;

	for (auto & wait_semaphore_info : wait_semaphore_infos) {
		wait_semaphore_handles.emplace_back(wait_semaphore_info.semaphore);
		wait_semaphore_stages.emplace_back(wait_semaphore_info.waiting_stage);
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphore_infos.size());
	submit_info.pWaitSemaphores = wait_semaphore_handles.data();
	submit_info.pWaitDstStageMask = wait_semaphore_stages.data();
	submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
	submit_info.pCommandBuffers = command_buffers.data();
	submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
	submit_info.pSignalSemaphores = signal_semaphores.data();

	VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
	if (VK_SUCCESS != result) {
		std::cout << "Error occurred during command buffer submission." << std::endl;
		return false;
	}
	return true;
}
*/
bool VulkanRenderer::wait_for_all_submitted_commands_to_be_finished(VkDevice logical_device)
{
	VkResult result = vkDeviceWaitIdle(logical_device);
	if (VK_SUCCESS != result) {
		std::cout << "Waiting on a device failed." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::create_buffer(
	VkDevice logical_device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkBuffer & buffer)
{
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.usage = usage;
	buffer_create_info.size = size;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = 0;
	buffer_create_info.pQueueFamilyIndices = nullptr;

	VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer));

	return true;
}


bool VulkanRenderer::allocate_and_bind_memory_object_to_buffer(
	VkPhysicalDevice physical_device,
	VkDevice logical_device,
	VkBuffer buffer,
	VkMemoryPropertyFlagBits memory_properties,
	VkDeviceMemory & memory_object)
{
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

	memory_object = VK_NULL_HANDLE;
	for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
		if ((memory_requirements.memoryTypeBits & (1 << type)) &&
			((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties)) {

			VkMemoryAllocateInfo buffer_memory_allocate_info = {};
			buffer_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			buffer_memory_allocate_info.allocationSize = memory_requirements.size;
			buffer_memory_allocate_info.memoryTypeIndex = type;

			VkResult result = vkAllocateMemory(logical_device, &buffer_memory_allocate_info, nullptr, &memory_object);
			if (VK_SUCCESS == result) {
				break;
			}
		}
	}

	if (VK_NULL_HANDLE == memory_object) {
		std::cout << "Could not allocate memory for a buffer." << std::endl;
		return false;
	}

	VkResult result = vkBindBufferMemory(logical_device, buffer, memory_object, 0);
	if (VK_SUCCESS != result) {
		std::cout << "Could not bind memory object to a buffer." << std::endl;
		return false;
	}
	return true;
}
/*
void VulkanRenderer::set_buffer_memory_barrier(
	VkCommandBuffer command_buffer,
	VkPipelineStageFlags generating_stages,
	VkPipelineStageFlags consuming_stages,
	std::vector<BufferTransition> buffer_transitions)
{

	std::vector<VkBufferMemoryBarrier> buffer_memory_barriers;

	for (auto & buffer_transition : buffer_transitions) {

		VkBufferMemoryBarrier buffer_memory_barrier = {};
		buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		buffer_memory_barrier.srcAccessMask = buffer_transition.current_access;
		buffer_memory_barrier.dstAccessMask = buffer_transition.new_access;
		buffer_memory_barrier.srcQueueFamilyIndex = buffer_transition.current_queue_family;
		buffer_memory_barrier.dstQueueFamilyIndex = buffer_transition.new_queue_family;
		buffer_memory_barrier.buffer = buffer_transition.buffer;
		buffer_memory_barrier.offset = 0;
		buffer_memory_barrier.size = VK_WHOLE_SIZE;
	}

	if (buffer_memory_barriers.size() > 0) {
		vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr, static_cast<uint32_t>(buffer_memory_barriers.size()), buffer_memory_barriers.data(), 0, nullptr);
	}
}
*/
bool VulkanRenderer::create_buffer_view(
	VkDevice logical_device,
	VkBuffer buffer,
	VkFormat format,
	VkDeviceSize memory_offset,
	VkDeviceSize memory_range,
	VkBufferView & buffer_view)
{
	VkBufferViewCreateInfo buffer_view_create_info = {};
	buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	//buffer_view_create_info.flags = 0;
	buffer_view_create_info.buffer = buffer;
	buffer_view_create_info.format = format;
	buffer_view_create_info.offset = memory_offset;
	buffer_view_create_info.range = memory_range;

	VK_CHECK_RESULT(vkCreateBufferView(logical_device, &buffer_view_create_info, nullptr, &buffer_view));

	return true;
}

bool VulkanRenderer::create_image(
	VkDevice logical_device,
	VkImageType type,
	VkFormat format,
	VkExtent3D size,
	uint32_t num_mipmaps,
	uint32_t num_layers,
	VkSampleCountFlagBits samples,
	VkImageUsageFlags usage_scenarios,
	bool cubemap,
	VkImage & image)
{
	VkImageCreateInfo image_create_info = {};

	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.flags = cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u;
	image_create_info.imageType = type;
	image_create_info.format = format;
	image_create_info.extent = size;
	image_create_info.mipLevels = num_mipmaps;
	image_create_info.arrayLayers = cubemap ? 6 * num_layers : num_layers;
	image_create_info.samples = samples;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = usage_scenarios;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = nullptr;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result = vkCreateImage(logical_device, &image_create_info, nullptr, &image);
	if (VK_SUCCESS != result) {
		std::cout << "Could not create an image." << std::endl;
		return false;
	}
	return true;
}
/*
void VulkanRenderer::set_image_memory_barrier(
	VkCommandBuffer command_buffer,
	VkPipelineStageFlags generating_stages,
	VkPipelineStageFlags consuming_stages,
	std::vector<ImageTransition> image_transitions)
{
	std::vector<VkImageMemoryBarrier> image_memory_barriers;

	for (auto & image_transition : image_transitions) {
		image_memory_barriers.push_back({
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // VkStructureType            sType
			nullptr,                                  // const void               * pNext
			image_transition.current_access,           // VkAccessFlags              srcAccessMask
			image_transition.new_access,               // VkAccessFlags              dstAccessMask
			image_transition.current_layout,           // VkImageLayout              oldLayout
			image_transition.new_layout,               // VkImageLayout              newLayout
			image_transition.current_queue_family,      // uint32_t                   srcQueueFamilyIndex
			image_transition.new_queue_family,          // uint32_t                   dstQueueFamilyIndex
			image_transition.image,                   // VkImage                    image
			{                                         // VkImageSubresourceRange    subresourceRange
				image_transition.aspect,                  // VkImageAspectFlags         aspectMask
				0,                                        // uint32_t                   baseMipLevel
				VK_REMAINING_MIP_LEVELS,                  // uint32_t                   levelCount
				0,                                        // uint32_t                   baseArrayLayer
				VK_REMAINING_ARRAY_LAYERS                 // uint32_t                   layerCount
			}
			});
	}

	if (image_memory_barriers.size() > 0) {
		vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(image_memory_barriers.size()), image_memory_barriers.data());
	}
}
*/