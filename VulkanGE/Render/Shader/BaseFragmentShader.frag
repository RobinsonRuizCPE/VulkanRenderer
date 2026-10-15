#version 450

layout(location = 0) in vec3 frag_color;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection_view_mat;
  vec3 direction_to_light;
}
ubo;

layout(push_constant) uniform Push {
  mat4 model_matric; // projection * view * model
  mat4 normal_matrix;
}
push;

void main() { out_color = vec4(frag_color, 1); }