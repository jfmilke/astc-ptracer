#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in flat float v_area[];

layout(location = 0) out vec3 normal;
layout(location = 1) out flat float area;

void main()
{
  vec3 line0 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 line1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 face_normal = cross(line0, line1);

  gl_Position = gl_in[0].gl_Position;
  normal = face_normal;
  area = v_area[0];
  EmitVertex();
  
  gl_Position = gl_in[1].gl_Position;
  normal = face_normal;
  area = v_area[1];
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  normal = face_normal;
  area = v_area[2];
  EmitVertex();
  
  EndPrimitive();
}