#pragma once

#include "VulkanRenderDevice.hpp"

#include <string>
#include <vector>

namespace VulkanRender {

	struct PipelineConfigInfo{


		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;

		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;

		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanRenderPipeline {
	public:
		VulkanRenderPipeline(
			VulkanRenderDevice& device,
			const std::string& vertex_file_path,
			const std::string& fragment_file_path,
			const PipelineConfigInfo& config_info);

		~VulkanRenderPipeline();

		VulkanRenderPipeline(const VulkanRenderPipeline&) = delete;
		VulkanRenderPipeline& operator=(const VulkanRenderPipeline&) = delete;


		void Bind(VkCommandBuffer command_buffer);


		static void DefaultPipelineConfigInfo(PipelineConfigInfo& config_info);

	private:
		static std::vector<char> ReadShaderFile(const std::string& file_path);

		void CreateGraphicsPipeline(const std::string& vertex_file_path, const std::string& fragment_file_path, const PipelineConfigInfo& config_info);
		void CreateShaderModule(std::vector<char>& code, VkShaderModule* shader_module);

		VulkanRenderDevice& render_device;
		VkPipeline graphics_pipeline;
		VkShaderModule vert_shader_module;
		VkShaderModule frag_shader_module;
	};
}