#pragma once

#include <GameObject/GameObject.hpp>
#include <VulkanWindow.hpp>

namespace Input {

	class Keyboard {

	public:
		struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
		};

        void MoveInPlaneXZ(GLFWwindow* window, float dt, Object::GameObject& game_object);

        KeyMappings keys{};
        float move_speed{ 3 };
        float turn_speed{ 1.5 };
	};
}