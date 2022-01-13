#version 450

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 final_color;

void main()
{
  final_color = color;
}