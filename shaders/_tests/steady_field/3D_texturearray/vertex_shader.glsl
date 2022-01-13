#version 450

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 velocity;

layout(location = 0) out vec4 color;

uniform float highlightThreshold;
uniform vec3  highlightColor;
uniform float damping;
uniform mat4  MVP;

void main()
{
  gl_Position   = MVP * vertex;

  float strength = length(velocity);
  color = vec4(0.1, 0.1, strength, damping);

  if (strength > highlightThreshold)
    color = vec4(highlightColor, 1.0);
}