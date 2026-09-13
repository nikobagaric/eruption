#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
  mat4 view;
  mat4 proj;
  vec4 lightPos;
  vec4 cameraPos;
  float time;
} ubo;

layout(location = 0) in vec3 inPosition;
// Unused here, but must stay declared: the shared vertex input layout
// (Buffer::Vertex) always binds these attributes, and validation layers
// flag any bound attribute the vertex shader doesn't consume.
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragDirection;

void main() {
  // Strip translation from the view matrix so the skybox is always centered
  // on the camera, then pin the resulting depth to the far plane (z == w,
  // i.e. 1.0 after the perspective divide). Combined with the skybox
  // pipeline's LEQUAL/no-write depth state, this lets it show through only
  // where no opaque geometry has drawn, regardless of draw order.
  mat4 viewRotationOnly = mat4(mat3(ubo.view));
  vec4 clipPos = ubo.proj * viewRotationOnly * vec4(inPosition, 1.0);
  gl_Position = clipPos.xyww;
  fragDirection = inPosition;
}
