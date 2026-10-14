#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

class Camera {
public:

	Camera();

	void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

	void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0,-1,0 });
	void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0,-1,0 });
	void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

	const glm::mat4& GetProjection() const { return projection_mat; }
	const glm::mat4& GetView() const { return view_mat; }


private:

	glm::mat4 projection_mat{ 1 };
	glm::mat4 view_mat{ 1 };

};