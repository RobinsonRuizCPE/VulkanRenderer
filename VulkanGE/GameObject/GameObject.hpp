#pragma once

#include <Render/Model/Model.hpp>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Object {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f,1.f,1.f };
		glm::vec3 rotation{};

		glm::mat4 mat4();
		glm::mat3 NormalMatrix();
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
		TransformComponent transform{};

	private:
		GameObject(id_t obj_id) : id{ obj_id } {}

		id_t id;

	};
}