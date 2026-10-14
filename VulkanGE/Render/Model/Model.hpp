#pragma once

#include "../VulkanRenderDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
namespace VulkanRender {

	class Model {

	public:

		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;
			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		};

		Model(VulkanRender::VulkanRenderDevice& device, const std::vector<Vertex>& vertices);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		void Bind(VkCommandBuffer command_buffer);
		void Draw(VkCommandBuffer command_buffer);


	private:
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		
		VulkanRender::VulkanRenderDevice& device;
		VkBuffer vertex_buffer;
		VkDeviceMemory vertex_buffer_memory;
		uint32_t vertex_count;
	};

} 