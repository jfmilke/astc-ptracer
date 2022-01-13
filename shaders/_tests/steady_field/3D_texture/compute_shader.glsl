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

layout(std430, binding = 2) buffer TestBuffer
{
  vec4 testvals[];
};

// NEW
layout(std140, binding = 0) uniform SeedingBuffer
{
  vec4  seeding_stride;
  uvec2 seeding_range_x;
  uvec2 seeding_range_y;
  uvec2 seeding_range_z;
};

// NEW
layout(std140, binding = 1) uniform IntegrationBuffer
{
  vec4  integration_stepsize;
  uint  integration_strategy;
  uint  integration_stepcount;
  uint  integration_texelfetch;
};

uniform sampler3D data;

// GLOBAL VARIABLES
// ================
vec2  one_zero  = vec2(1.0, 0.0);
vec3  tex_size  = textureSize(data, 0) - vec3(1, 1, 1);

// METHODS
//========
// Manual trilinear interpolation
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

// TEXTURE BASED VELOCITY
// ======================
// Improved Euler Scheme (Texture)
vec4 update_velocity_euler_texture(vec4 l_pos, vec4 stepsize)
{
  /*
  // Left slope
  vec3 l_coord = l_pos.xyz / tex_size;
  vec4 l_velo  = texture(data, l_coord, 0);

  // Right slope
  vec3 r_coord = (l_pos + stepsize * l_velo).xyz / tex_size;
  vec4 r_velo  = texture(data, r_coord, 0);

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
  */
  vec4  k1 =texture(data, l_pos.xyz / tex_size);
  vec4  k2 =texture(data, (l_pos + stepsize * (k1/2.0)).xyz / tex_size);
  vec4  k3 =texture(data, (l_pos + stepsize * (k2/2.0)).xyz / tex_size);
  vec4  k4 =texture(data, (l_pos + stepsize *  k3     ).xyz / tex_size);

  // Multiply with (step_size/6.0)
  vec4  velo_rk4 = k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Runge Kutta 4th order (Texture)
vec4 update_velocity_rk4_texture(vec4 y0, vec4 stepsize)
{
  /*
  vec4  k1 = velo_from_texel(data, y0.xyz);
  vec4  k2 = velo_from_texel(data, (y0 + stepsize * (k1/2.0)).xyz);
  vec4  k3 = velo_from_texel(data, (y0 + stepsize * (k2/2.0)).xyz);
  vec4  k4 = velo_from_texel(data, (y0 + stepsize *  k3     ).xyz);
  */

  vec4  k1 = update_velocity_euler_texture(y0, stepsize);
  vec4  k2 = update_velocity_euler_texture(y0 + stepsize * (k1/2.0), stepsize);
  vec4  k3 = update_velocity_euler_texture(y0 + stepsize * (k2/2.0), stepsize);
  vec4  k4 = update_velocity_euler_texture(y0 + stepsize *  k3     , stepsize);

  // Multiply with (step_size/6.0)
  vec4  velo_rk4 = k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;

  return velo_rk4 * one_zero.xxxy;
}

// TEXELFETCH BASED VELOCITY
// =========================
// Improved Euler Scheme (Texel)
vec4 update_velocity_euler_texel(vec4 l_pos, vec4 stepsize)
{
  /*
  // Left slope
  vec4 l_velo  = velo_from_texel(data, l_pos.xyz);

  // Right slope
  vec4 r_pos   = l_pos + stepsize * l_velo;
  vec4 r_velo  = velo_from_texel(data, r_pos.xyz);

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
  */

  vec4  k1 = velo_from_texel(data, l_pos.xyz);
  vec4  k2 = velo_from_texel(data, (l_pos + stepsize * (k1/2.0)).xyz);
  vec4  k3 = velo_from_texel(data, (l_pos + stepsize * (k2/2.0)).xyz);
  vec4  k4 = velo_from_texel(data, (l_pos + stepsize *  k3     ).xyz);

  // Multiply with (step_size/6.0)
  vec4  velo_rk4 = k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Runge Kutta 4th order (Texel)
vec4 update_velocity_rk4_texel(vec4 y0, vec4 stepsize)
{
  /*
  vec4  k1 = velo_from_texel(data, y0.xyz);
  vec4  k2 = velo_from_texel(data, (y0 + stepsize * (k1/2.0)).xyz);
  vec4  k3 = velo_from_texel(data, (y0 + stepsize * (k2/2.0)).xyz);
  vec4  k4 = velo_from_texel(data, (y0 + stepsize *  k3     ).xyz);
  */

  vec4  k1 = update_velocity_euler_texel(y0, stepsize);
  vec4  k2 = update_velocity_euler_texel(y0 + stepsize * (k1/2.0), stepsize);
  vec4  k3 = update_velocity_euler_texel(y0 + stepsize * (k2/2.0), stepsize);
  vec4  k4 = update_velocity_euler_texel(y0 + stepsize *  k3     , stepsize);

  // Multiply with (step_size/6.0)
  vec4  velo_rk4 = k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;

  return velo_rk4 * one_zero.xxxy;
}

vec4 update_position(vec4 pos, vec4 velo, vec4 stepsize)
{
  return (pos + stepsize * velo);
}

// MAIN
//=====
void main()
{
  uint id = uint(gl_GlobalInvocationID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x) * integration_stepcount;

  uvec3 seeding_offset = uvec3(seeding_range_x.x, seeding_range_y.x, seeding_range_z.x);

  vec4 pos  = vec4(gl_GlobalInvocationID.xyz * seeding_stride.xyz + seeding_offset, 1);
  vec4 velo = vec4(0,0,0,0);

  vec4 rk4_step_size = integration_stepsize / 6.0;
  
  // seeding_stride.w
  // integration_strategy
  if (id == 0)
  {
    testvals[0] = vec4(seeding_stride);
    testvals[1] = vec4(seeding_range_x, 0, 0);
    testvals[2] = vec4(seeding_range_y, 0, 0);
    testvals[3] = vec4(seeding_range_z, 0, 0);
    testvals[4] = vec4(integration_stepsize);
    testvals[5] = vec4(integration_strategy, integration_stepcount, integration_texelfetch, 0);
    testvals[6] = vec4(0, 0, 0, 0);
    testvals[7] = vec4(0, 0, 0, 0);

    /*
    testvals[0] = texelFetch(data, ivec3(0,0,0)           , 0);
    testvals[1] = texture   (data,  vec3(0,0,0) / tex_size, 0);
    testvals[2] = texelFetch(data, ivec3(1,0,0)           , 0);
    testvals[3] = texture   (data,  vec3(1,0,0) / tex_size, 0);
    testvals[4] = texelFetch(data, ivec3(13,20,0)           , 0);
    testvals[5] = texture   (data,  vec3(13,20,0) / tex_size, 0);
    testvals[6] = texelFetch(data, ivec3(60,70,3)           , 0);
    testvals[7] = texture   (data,  vec3(60,70,3) / tex_size, 0);
    testvals[8] = texelFetch(data, ivec3(1,1,1)           , 0);
    testvals[9] = texture   (data,  vec3(1,1,1) / tex_size, 0);
    testvals[10]= vec4(0, 0, 0, 0);
    */
  }

  if (integration_texelfetch == 0)
  {
    testvals[0] = vec4(0, 0, 0, 0);
    if (integration_strategy == 0)
      for (int i = 0; i < integration_stepcount; i++)
      {
        pos  = update_position(pos, velo, integration_stepsize);
        velo = update_velocity_euler_texture(pos, integration_stepsize);  // Euler + Texelfetch
        positions[id + i]  = pos;
        velocities[id + i] = velo;
      }
    else
      for (int i = 0; i < integration_stepcount; i++)
      {
        pos  = update_position(pos, velo, integration_stepsize);
        velo = update_velocity_rk4_texture(pos, integration_stepsize);    // RK4 + Texelfetch
        positions[id + i]  = pos;
        velocities[id + i] = velo;
      }
  }
  else
  {
    testvals[1] = vec4(1, 1, 1, 1);
    if (integration_strategy == 0)
      for (int i = 0; i < integration_stepcount; i++)
      {
        pos  = update_position(pos, velo, integration_stepsize);
        velo = update_velocity_euler_texel(pos, integration_stepsize);    // Euler + Texture
        positions[id + i]  = pos;
        velocities[id + i] = velo;
      }
    else
      for (int i = 0; i < integration_stepcount; i++)
      {
        pos  = update_position(pos, velo, integration_stepsize);
        velo = update_velocity_rk4_texel(pos, integration_stepsize);      // RK4 + Texture
        positions[id + i]  = pos;
        velocities[id + i] = velo;
      }
  }
}