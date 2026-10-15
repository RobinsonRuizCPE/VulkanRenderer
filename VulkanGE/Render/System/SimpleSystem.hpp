#pragma once

#include <Render/VulkanRenderPipeline.hpp>
#include <Render/Model/Model.hpp>
#include <GameObject/GameObject.hpp>
#include <Camera/Camera.hpp>
#include <Render/Helpers/FrameInfo.hpp>

#include <memory>
#include <vector>

namespace VulkanRender {

	class SimpleSystem {
	public:

		SimpleSystem(VulkanRenderDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~SimpleSystem();

		SimpleSystem(const SimpleSystem&) = delete;
		SimpleSystem& operator=(const SimpleSystem&) = delete;

		void RenderGameObjects(FrameInfo& frame_info, std::vector<Object::GameObject>& game_objects);


	private:
		void CreatePipelineLayout(VkDescriptorSetLayout global_set_layout);
		void CreatePipeline(VkRenderPass render_pass);

		VulkanRenderDevice& vulkan_render_device;
		std::unique_ptr<VulkanRender::VulkanRenderPipeline> vulkan_render_pipeline;
		VkPipelineLayout pipeline_layout;
	};
}