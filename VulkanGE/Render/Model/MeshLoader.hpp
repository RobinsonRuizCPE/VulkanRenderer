#pragma once

#include "Model.hpp"

#include <string>

namespace VulkanRender {

	struct MeshLoadOptions {
		float scale = 1.0f;
		bool flip_y = true;
		glm::vec3 default_color{ 1.0f, 1.0f, 1.0f };
		bool flip_uv_v = false;
		bool generate_missing_normals = true;
		bool deduplicate_vertices = true;
	};

	class MeshLoader {
	public:

		using LoadOptions = MeshLoadOptions;

		static bool LoadFromFile(
			const std::string& path,
			Model::Builder& out,
			const MeshLoadOptions& options = MeshLoadOptions{},
			std::string* error = nullptr);

		static bool LoadFbx(
			const std::string& path,
			Model::Builder& out,
			const MeshLoadOptions& options = MeshLoadOptions{},
			std::string* error = nullptr);

		static bool LoadGltf(
			const std::string& path,
			Model::Builder& out,
			const MeshLoadOptions& options = MeshLoadOptions{},
			std::string* error = nullptr);
	};

}
