#version 450

layout(location = 0) in vec4 vertex;

layout(std430, binding = 0) buffer area_buffer
{
  double area[];
};

layout(std430, binding = 1) buffer area_id_buffer
{
  uint area_id[];
};

layout(location = 0) out vec4 color;

uniform mat4 View;
uniform mat4 Proj;
uniform vec3 cam_pos;

void main()
{
  gl_Position = Proj * View * vertex;
  uint index = area_id[gl_VertexID];
  
  if (area[index] > 1400.0)
    color = vec4(1.0, 0.0, 0.0, 1.0);
  else
    color = vec4(0.0, 0.6, 0.0, 0.2);
}