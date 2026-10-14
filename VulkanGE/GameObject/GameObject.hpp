#pragma once

#include <Render/Model/Model.hpp>
#include <glm/glm.hpp>

#include <memory>

namespace Object {

	struct Transform2DComponent {
		glm::vec2 translation{};
		glm::vec2 scale{ 1.f,1.f };
		float rotation;
		glm::mat2 mat2() { 
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rot_mat = { {c,s},{-s,c} };
			glm::mat2 scale_mat{ {scale.x, 0.f}, {0.f, scale.y} };
			return rot_mat * scale_mat;
		}
	};

	class GameObject {
	public:

		using id_t = unsigned int;

		static GameObject CreateGameObject() {
			static id_t current_id = 0;
			return GameObject{ current_id++ };
		}

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;

		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		id_t GetId() const { return id; }

		std::shared_ptr<VulkanRender::Model> model{};
		glm::vec3 color{};
		Transform2DComponent transform_2d{};

	private:
		GameObject(id_t obj_id) : id{ obj_id } {}

		id_t id;

	};
}