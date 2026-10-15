#pragma once

#include <Camera/Camera.hpp>

#include <vulkan/vulkan.h>

namespace VulkanRender {

	struct FrameInfo {
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		Camera& camera;
		VkDescriptorSet global_descriptor_set;
	};
}