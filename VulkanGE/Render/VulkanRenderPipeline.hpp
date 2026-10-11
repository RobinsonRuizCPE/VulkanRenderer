#pragma once

#include "VulkanRenderDevice.hpp"

#include <string>
#include <vector>

namespace VulkanRender {

	struct PipelineConfigInfo{
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
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
		void operator=(const VulkanRenderPipeline&) = delete;


		void Bind(VkCommandBuffer command_buffer);


		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);

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