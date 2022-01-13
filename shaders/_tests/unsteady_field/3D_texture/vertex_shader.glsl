#version 450

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 velocity;

layout(location = 0) out vec3 color;

uniform float highlightThreshold;
uniform vec3  highlightColor;
uniform float damping;
uniform mat4  MVP;

void main()
{
  gl_Position   = MVP * vertex;

  float strength = length(velocity);
  color = vec3(0.1,0.1, damping * strength);

  if (strength > highlightThreshold)
    color = highlightColor;
}