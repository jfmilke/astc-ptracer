#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer PositionBuffer
{
  vec4 positions[];
};

layout(std430, binding = 1) buffer VelocityBuffer
{
  vec4 velocities[];
};

layout(std140, binding = 0) uniform ReconstructionBuffer
{
  uvec4 grid;
  uvec4 seeding_stride;
  uvec4 seeding_offset0;
  uvec4 seeding_offset1;
  uint  global_steps;        // globally processed: global_steps   = time_dimension * internal_steps
  uint  internal_steps;      // 1 GPU invocation:   internal_steps = steps / time_dimension
  uint  iterations_per_step; // sub steps:          1 step         = iterations_per_step
};

layout (binding = 1) uniform sampler3D data1;
layout (binding = 2) uniform sampler3D data2;

uniform uint  global_time;

// GLOBAL VARIABLES
// ================
vec2  one_zero  = vec2(1.0, 0.0);
float step_size = 1.0 / float(iterations_per_step);


// METHODS
//========
// Manual trilinear interpolation (most precise)
//     c3--------c2           d3--------d2
//      | s       | lambda     | s       |
//      |---c     |----P       |---d     |
//      |   | t   |            |   | t   |
//     c0--------c1           d0--------d1
//     c0 := P0               d2 := P1
vec4 velo_from_texel(sampler3D tex, vec3 P)
{
  // Reference points to resemble all interpolated texels
  ivec4 P0 = ivec4(floor(P), 0);
  ivec4 P1 = ivec4(floor(P), 0) + ivec4(1,1,1,0);

  // Factors for trilinear interpolation
  float s      = fract(P.x);
  float t      = fract(P.y);
  float lambda = fract(P.z);

  // Lower depth
  vec4 c0 = texelFetch(tex, P0.xyz,          0);
  vec4 c1 = texelFetch(tex, P0.wyz + P1.xww, 0);
  vec4 c2 = texelFetch(tex, P0.wwz + P1.xyw, 0);
  vec4 c3 = texelFetch(tex, P0.xwz + P1.wyw, 0);

  // Higher depth
  vec4 d0 = texelFetch(tex, P0.xyw + P1.wwz, 0);
  vec4 d1 = texelFetch(tex, P0.wyw + P1.xwz, 0);
  vec4 d2 = texelFetch(tex,          P1.xyz, 0);
  vec4 d3 = texelFetch(tex, P0.xww + P1.wyz, 0);

  // Bilinear interpolation (2D, near)
  vec4 i0 = mix(c0, c1, s);
  vec4 i1 = mix(c3, c2, s);
  vec4 c  = mix(i0, i1, t);

  // Bilinear interpolation (2D, far)
  vec4 j0 = mix(d0, d1, s);
  vec4 j1 = mix(d3, d2, s);
  vec4 d  = mix(j0, j1, t);

  // Trilinear interpolation (3D)
  return mix(c, d, lambda);
}


// Improved Euler Scheme
// Manual trilinear interpolation using the real position is significantly more precise,
// since normalization for texture(...)-access introduces huge rounding errors.
vec4 update_velocity(vec4 l_pos, sampler3D texture, float time)
{
  // Left slope
  vec4 l_velo  = velo_from_texel(texture, l_pos.xyz);

  // Right slope
  vec4 r_pos   = l_pos + time * l_velo;
  vec4 r_velo  = velo_from_texel(texture, r_pos.xyz);

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

vec4 update_position(vec4 pos, vec4 velo, float time)
{
  return pos + time * velo;
}

// Time Interpolation:
//====================
// Manual interpolation (most precise)
// (2x trilinear interpolation)
//     c3--------c2           d3--------d2
//      | s       | lambda     | s       |
//      |---c     |----P       |---d     |    data_0 (timestep i)
//      |   | t   |    |       |   | t   |
//     c0--------c1    |      d0--------d1
//                     X      
//     e3--------e2    |time  f3--------f2
//      | s       |    |       | s       |
//      |---e     |----P       |---f     |    data_1 (timestep i + 1)
//      |   | t   | lambda     |   | t   |
//     e0--------e1           f0--------f1
//

// MAIN
//=====
void main()
{
  uint id       = uint(gl_GlobalInvocationID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x) * global_steps * iterations_per_step;
  uint id_offs  = uint(global_time * internal_steps * iterations_per_step);

  float time_per_iteration = 1.0 / float(internal_steps * iterations_per_step);

  // Get the results of the last invocation
  vec4 pos   = positions[ id + id_offs - 1];
  vec4 velo  = velocities[id + id_offs - 1];
  vec4 velo1 = vec4(0,0,0,0);
  vec4 velo2 = vec4(0,0,0,0);

  if (global_time == 0U)
  {
    // Initialize once
    pos   = vec4(gl_GlobalInvocationID.xyz * seeding_stride.xyz + seeding_offset0.xyz, 1);
    velo  = vec4(0,0,0,0);
  }

  // Perform all steps for a timeslice
  for (int i = 0; i < internal_steps * iterations_per_step; i++)
  {
    // For interpolation between the two timesteps (in range [0.0, 1.0])
    float local_time = i * time_per_iteration;
    float integrate_by = 0.1;

    // Update postiion with the last calculated velocity
    pos   = update_position(pos, velo, integrate_by);
    // Calculate temporary velocity of both timesteps
    velo1 = update_velocity(pos, data1, integrate_by);
    velo2 = update_velocity(pos, data2, integrate_by);
    // Interpolate
    velo  = mix(velo1, velo2, local_time);
    // Save the position and the (interpolated) velocity at this position
    positions[id + id_offs +  i]  = pos;
    velocities[id + id_offs + i] = velo;
  }
}