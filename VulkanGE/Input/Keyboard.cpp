#include "Keyboard.hpp"

namespace Input {

	void Keyboard::MoveInPlaneXZ(GLFWwindow* window, float dt, Object::GameObject& game_object)
	{
		glm::vec3 rotate{ 0 };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			game_object.transform.rotation += move_speed * dt * glm::normalize(rotate);
		}

		game_object.transform.rotation.x = glm::clamp(game_object.transform.rotation.x, -1.5f, 1.5f);
		game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y, glm::two_pi<float>());

		float yaw = game_object.transform.rotation.y;
		const glm::vec3 forward_dir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 right_dir{ forward_dir.z, 0.f, -forward_dir.x };
		const glm::vec3 up_dir{ 0, -1.f, 0.f };

		glm::vec3 move_dir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) move_dir += forward_dir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) move_dir -= forward_dir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) move_dir += right_dir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) move_dir -= right_dir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) move_dir += up_dir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) move_dir -= up_dir;


		if (glm::dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon()) {
			game_object.transform.translation += move_speed * dt * glm::normalize(move_dir);
		}
	}

}