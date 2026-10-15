#pragma once

#include "../VulkanRenderDevice.hpp"
#include <Render/Helpers/Buffer.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace VulkanRender {

	class Model {

	public:

		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};
			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void LoadModel(const std::string& file_path);
		};

		Model(VulkanRender::VulkanRenderDevice& device, const Builder& builder);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		static std::unique_ptr<Model> CreateModelfromFile(VulkanRenderDevice& device, const std::string& file_path);

		void Bind(VkCommandBuffer command_buffer);
		void Draw(VkCommandBuffer command_buffer);


	private:
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);

		VulkanRender::VulkanRenderDevice& device;

		std::unique_ptr<Buffer> vertex_buffer;
		uint32_t vertex_count;

		bool has_index_buffer;
		std::unique_ptr<Buffer> index_buffer;

		uint32_t index_count;
	};

} 