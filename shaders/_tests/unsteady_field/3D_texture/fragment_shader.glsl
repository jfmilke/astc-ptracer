#version 450

layout(location = 0) in vec3 color ;

layout(location = 0) out vec3 final_color;

void main()
{
  final_color = color;
}