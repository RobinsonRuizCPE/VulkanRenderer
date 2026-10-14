#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Push {
  mat4 transform; // projection * view * model
  mat4 normal_matrix;
}
push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT_LIGHT = 0.02;

void main() {
  gl_Position = push.transform * vec4(position, 1.0);

  vec3 normal_world_space = normalize(mat3(push.normal_matrix) * normal);

  float light_intensity =
      AMBIENT_LIGHT + max(dot(normal_world_space, DIRECTION_TO_LIGHT), 0);
  frag_color = light_intensity * color;
}