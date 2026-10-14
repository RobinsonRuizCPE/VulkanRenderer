#include "SimpleSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

struct SimplePushConstantData {
	glm::mat2 transform{ 1.f };
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

namespace VulkanRender {

	SimpleSystem::SimpleSystem(VulkanRenderDevice& device, VkRenderPass render_pass) : vulkan_render_device{device}
	{
		CreatePipelineLayout();
		CreatePipeline(render_pass);
	}

	SimpleSystem::~SimpleSystem()
	{
		vkDestroyPipelineLayout(vulkan_render_device.device(), pipeline_layout, nullptr);
	}

	void SimpleSystem::CreatePipelineLayout()
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);


		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(vulkan_render_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
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

	void SimpleSystem::RenderGameObjects(VkCommandBuffer command_buffer, std::vector<Object::GameObject>& game_objects)
	{
		vulkan_render_pipeline->Bind(command_buffer);
		for (auto& object : game_objects) {
			SimplePushConstantData push{};
			push.offset = object.transform_2d.translation;
			push.color = object.color;
			push.transform = object.transform_2d.mat2();

			vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			object.model->Bind(command_buffer);
			object.model->Draw(command_buffer);
		}
	}
	
}