#include "SimpleSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

struct SimplePushConstantData {
	glm::mat4 model_matrix{ 1.f };
	glm::mat4 normal_matrix{ 1.f };
};

namespace VulkanRender {

	SimpleSystem::SimpleSystem(VulkanRenderDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : vulkan_render_device{device}
	{
		CreatePipelineLayout(global_set_layout);
		CreatePipeline(render_pass);
	}

	SimpleSystem::~SimpleSystem()
	{
		vkDestroyPipelineLayout(vulkan_render_device.GetDevice(), pipeline_layout, nullptr);
	}

	void SimpleSystem::CreatePipelineLayout(VkDescriptorSetLayout global_set_layout)
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptor_set_layout{ global_set_layout };

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layout.size());
		pipeline_layout_info.pSetLayouts = descriptor_set_layout.data();
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(vulkan_render_device.GetDevice(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void SimpleSystem::CreatePipeline(VkRenderPass render_pass)
	{
		assert(pipeline_layout != nullptr && "Cannot create pipeline before piepline layout");

		VulkanRender::PipelineConfigInfo pipeline_config{};
		VulkanRender::VulkanRenderPipeline::DefaultPipelineConfigInfo(pipeline_config);
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout;
		vulkan_render_pipeline = std::make_unique<VulkanRender::VulkanRenderPipeline>(vulkan_render_device, "Render/Shader/BaseVertexShader.vert.spv", "Render/Shader/BaseFragmentShader.frag.spv", pipeline_config);

	}

	void SimpleSystem::RenderGameObjects(FrameInfo& frame_info, std::vector<Object::GameObject>& game_objects)
	{
		vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &frame_info.global_descriptor_set, 0, nullptr);
		vulkan_render_pipeline->Bind(frame_info.command_buffer);
		for (auto& object : game_objects) {
			SimplePushConstantData push{};
			push.normal_matrix = object.transform.NormalMatrix();
			push.model_matrix = object.transform.mat4();

			vkCmdPushConstants(frame_info.command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			object.model->Bind(frame_info.command_buffer);
			object.model->Draw(frame_info.command_buffer);
		}
	}
	
}