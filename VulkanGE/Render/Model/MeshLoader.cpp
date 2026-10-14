#include "MeshLoader.hpp"

#include <ufbx.h>

#include <cgltf.h>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <unordered_map>

namespace VulkanRender {

	namespace {
		void SetError(std::string* error, std::string message)
		{
			if (error) {
				*error = std::move(message);
			}
		}

		std::string ToLowerExtension(const std::string& path)
		{
			const size_t dot = path.find_last_of('.');
			if (dot == std::string::npos) {
				return {};
			}

			std::string ext = path.substr(dot + 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
				});
			return ext;
		}

		class VertexAccumulator {
		public:
			explicit VertexAccumulator(bool deduplicate) : deduplicate{ deduplicate } {}

			void Add(const Model::Vertex& vertex)
			{
				if (!deduplicate) {
					builder.indices.push_back(static_cast<uint32_t>(builder.vertices.size()));
					builder.vertices.push_back(vertex);
					return;
				}

				const auto it = lookup.find(vertex);
				if (it != lookup.end()) {
					builder.indices.push_back(it->second);
					return;
				}

				const uint32_t index = static_cast<uint32_t>(builder.vertices.size());
				lookup.emplace(vertex, index);
				builder.vertices.push_back(vertex);
				builder.indices.push_back(index);
			}

			Model::Builder& Result() { return builder; }

		private:
			struct VertexEqual {
				bool operator()(const Model::Vertex& a, const Model::Vertex& b) const
				{
					return a.position == b.position
						&& a.color == b.color
						&& a.normal == b.normal
						&& a.uv == b.uv;
				}
			};

			struct VertexHash {
				size_t operator()(const Model::Vertex& vertex) const
				{
					const float components[11] = {
						vertex.position.x, vertex.position.y, vertex.position.z,
						vertex.color.x,    vertex.color.y,    vertex.color.z,
						vertex.normal.x,   vertex.normal.y,   vertex.normal.z,
						vertex.uv.x,       vertex.uv.y,
					};

					size_t seed = 0;
					for (float component : components) {
						if (component == 0.0f) {
							component = 0.0f;
						}
						uint32_t bits;
						std::memcpy(&bits, &component, sizeof(bits));
						seed ^= static_cast<size_t>(bits) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
					}
					return seed;
				}
			};

			bool deduplicate;
			Model::Builder builder{};
			std::unordered_map<Model::Vertex, uint32_t, VertexHash, VertexEqual> lookup{};
		};

		glm::vec3 ApplyPositionOptions(glm::vec3 position, const MeshLoadOptions& options)
		{
			position *= options.scale;
			if (options.flip_y) {
				position.y = -position.y;
			}
			return position;
		}

		glm::vec3 ApplyNormalOptions(glm::vec3 normal, const MeshLoadOptions& options)
		{
			if (options.flip_y) {
				normal.y = -normal.y;
			}

			const float length_squared = glm::dot(normal, normal);
			if (length_squared > 0.0f) {
				normal /= std::sqrt(length_squared);
			}
			return normal;
		}

		glm::vec2 ApplyUvOptions(glm::vec2 uv, const MeshLoadOptions& options, bool already_top_left)
		{
			const bool flip = (already_top_left ? false : true) != options.flip_uv_v;
			if (flip) {
				uv.y = 1.0f - uv.y;
			}
			return uv;
		}

	}

	bool MeshLoader::LoadFromFile(
		const std::string& path,
		Model::Builder& out,
		const MeshLoadOptions& options,
		std::string* error)
	{
		const std::string ext = ToLowerExtension(path);

		if (ext == "fbx") {
			return LoadFbx(path, out, options, error);
		}
		if (ext == "gltf" || ext == "glb") {
			return LoadGltf(path, out, options, error);
		}

		SetError(error, "Unsupported mesh format '" + ext + "' for file: " + path);
		return false;
	}

	// -------------------------------------------------------------------
	// FBX (ufbx)
	// -------------------------------------------------------------------

	bool MeshLoader::LoadFbx(
		const std::string& path,
		Model::Builder& out,
		const MeshLoadOptions& options,
		std::string* error)
	{
		ufbx_load_opts load_opts{};
		load_opts.target_axes = ufbx_axes_right_handed_y_up;
		load_opts.target_unit_meters = 1.0f;
		load_opts.space_conversion = UFBX_SPACE_CONVERSION_TRANSFORM_ROOT;
		load_opts.generate_missing_normals = options.generate_missing_normals;

		ufbx_error ufbx_err;
		ufbx_scene* scene = ufbx_load_file(path.c_str(), &load_opts, &ufbx_err);
		if (!scene) {
			char buffer[512];
			ufbx_format_error(buffer, sizeof(buffer), &ufbx_err);
			SetError(error, "ufbx failed to load '" + path + "': " + buffer);
			return false;
		}

		VertexAccumulator accumulator{ options.deduplicate_vertices };
		std::vector<uint32_t> triangle_indices;

		for (size_t node_index = 0; node_index < scene->nodes.count; node_index++) {
			const ufbx_node* node = scene->nodes.data[node_index];
			if (node->is_root || !node->mesh) {
				continue;
			}

			const ufbx_mesh* mesh = node->mesh;
			const ufbx_matrix normal_matrix = ufbx_matrix_for_normals(&node->geometry_to_world);
			triangle_indices.resize(static_cast<size_t>(mesh->max_face_triangles) * 3);

			for (size_t face_index = 0; face_index < mesh->faces.count; face_index++) {
				const ufbx_face face = mesh->faces.data[face_index];

				const uint32_t triangle_count = ufbx_triangulate_face(
					triangle_indices.data(),
					triangle_indices.size(),
					mesh,
					face);

				for (uint32_t corner = 0; corner < triangle_count * 3; corner++) {
					const uint32_t vertex_index = triangle_indices[corner];

					Model::Vertex vertex{};

					const ufbx_vec3 local_position =
						ufbx_get_vertex_vec3(&mesh->vertex_position, vertex_index);
					const ufbx_vec3 world_position =
						ufbx_transform_position(&node->geometry_to_world, local_position);
					vertex.position = ApplyPositionOptions(
						glm::vec3{
							static_cast<float>(world_position.x),
							static_cast<float>(world_position.y),
							static_cast<float>(world_position.z) },
						options);

					if (mesh->vertex_color.exists) {
						const ufbx_vec4 color =
							ufbx_get_vertex_vec4(&mesh->vertex_color, vertex_index);
						vertex.color = glm::vec3{
							static_cast<float>(color.x),
							static_cast<float>(color.y),
							static_cast<float>(color.z) };
					}
					else {
						vertex.color = options.default_color;
					}

					if (mesh->vertex_normal.exists) {
						const ufbx_vec3 local_normal =
							ufbx_get_vertex_vec3(&mesh->vertex_normal, vertex_index);
						const ufbx_vec3 world_normal =
							ufbx_transform_direction(&normal_matrix, local_normal);
						vertex.normal = ApplyNormalOptions(
							glm::vec3{
								static_cast<float>(world_normal.x),
								static_cast<float>(world_normal.y),
								static_cast<float>(world_normal.z) },
							options);
					}

					if (mesh->vertex_uv.exists) {
						const ufbx_vec2 uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, vertex_index);
						vertex.uv = ApplyUvOptions(
							glm::vec2{ static_cast<float>(uv.x), static_cast<float>(uv.y) },
							options,
							false);
					}

					accumulator.Add(vertex);
				}
			}
		}

		ufbx_free_scene(scene);

		if (accumulator.Result().vertices.empty()) {
			SetError(error, "No mesh data found in: " + path);
			return false;
		}

		out = std::move(accumulator.Result());
		return true;
	}

	bool MeshLoader::LoadGltf(
		const std::string& path,
		Model::Builder& out,
		const MeshLoadOptions& options,
		std::string* error)
	{
		cgltf_options parse_options{};
		cgltf_data* data = nullptr;

		cgltf_result result = cgltf_parse_file(&parse_options, path.c_str(), &data);
		if (result != cgltf_result_success) {
			SetError(error, "cgltf failed to parse: " + path);
			return false;
		}

		result = cgltf_load_buffers(&parse_options, data, path.c_str());
		if (result != cgltf_result_success) {
			cgltf_free(data);
			SetError(error, "cgltf failed to load buffers for: " + path);
			return false;
		}

		if (cgltf_validate(data) != cgltf_result_success) {
			cgltf_free(data);
			SetError(error, "cgltf validation failed for: " + path);
			return false;
		}

		VertexAccumulator accumulator{ options.deduplicate_vertices };

		for (size_t node_index = 0; node_index < data->nodes_count; node_index++) {
			const cgltf_node& node = data->nodes[node_index];
			if (!node.mesh) {
				continue;
			}

			float world_matrix[16];
			cgltf_node_transform_world(&node, world_matrix);
			const glm::mat4 node_to_world = glm::make_mat4(world_matrix);
			const glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3{ node_to_world }));

			for (size_t primitive_index = 0;
				primitive_index < node.mesh->primitives_count;
				primitive_index++) {

				const cgltf_primitive& primitive = node.mesh->primitives[primitive_index];
				if (primitive.type != cgltf_primitive_type_triangles) {
					continue;
				}

				const cgltf_accessor* position_accessor = nullptr;
				const cgltf_accessor* color_accessor = nullptr;
				const cgltf_accessor* normal_accessor = nullptr;
				const cgltf_accessor* uv_accessor = nullptr;

				for (size_t attribute_index = 0;
					attribute_index < primitive.attributes_count;
					attribute_index++) {

					const cgltf_attribute& attribute = primitive.attributes[attribute_index];
					switch (attribute.type) {
					case cgltf_attribute_type_position:
						position_accessor = attribute.data;
						break;
					case cgltf_attribute_type_normal:
						normal_accessor = attribute.data;
						break;
					case cgltf_attribute_type_color:
						if (attribute.index == 0) color_accessor = attribute.data;
						break;
					case cgltf_attribute_type_texcoord:
						if (attribute.index == 0) uv_accessor = attribute.data;
						break;
					default:
						break;
					}
				}

				if (!position_accessor) {
					continue;
				}

				const size_t corner_count = primitive.indices
					? primitive.indices->count
					: position_accessor->count;

				for (size_t corner = 0; corner < corner_count; corner++) {
					const size_t vertex_index = primitive.indices
						? cgltf_accessor_read_index(primitive.indices, corner)
						: corner;

					Model::Vertex vertex{};

					float position[3] = { 0.0f, 0.0f, 0.0f };
					cgltf_accessor_read_float(position_accessor, vertex_index, position, 3);
					const glm::vec3 world_position = glm::vec3{
						node_to_world * glm::vec4{ position[0], position[1], position[2], 1.0f } };
					vertex.position = ApplyPositionOptions(world_position, options);

					if (color_accessor) {
						float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						cgltf_accessor_read_float(color_accessor, vertex_index, color, 4);
						vertex.color = glm::vec3{ color[0], color[1], color[2] };
					}
					else {
						vertex.color = options.default_color;
					}

					if (normal_accessor) {
						float normal[3] = { 0.0f, 0.0f, 0.0f };
						cgltf_accessor_read_float(normal_accessor, vertex_index, normal, 3);
						vertex.normal = ApplyNormalOptions(
							normal_matrix * glm::vec3{ normal[0], normal[1], normal[2] },
							options);
					}

					if (uv_accessor) {
						float uv[2] = { 0.0f, 0.0f };
						cgltf_accessor_read_float(uv_accessor, vertex_index, uv, 2);
						vertex.uv = ApplyUvOptions(
							glm::vec2{ uv[0], uv[1] },
							options,
							true);
					}

					accumulator.Add(vertex);
				}
			}
		}

		cgltf_free(data);

		if (accumulator.Result().vertices.empty()) {
			SetError(error, "No triangle mesh data found in: " + path);
			return false;
		}

		out = std::move(accumulator.Result());
		return true;
	}

}
