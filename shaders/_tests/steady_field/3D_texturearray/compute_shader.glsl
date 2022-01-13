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

layout(std430, binding = 2) buffer PeaksBuffer
{
  float peaks[];
};

layout(std140, binding = 0) uniform ReconstructionBuffer
{
  uvec4 grid;
  uvec4 seeding_stride;
  uvec4 seeding_offset0;
  uvec4 seeding_offset1;
  uint  steps;
  uint  iterations_per_step; // sub steps: 1 step = iterations_per_step
  uint channels;
  uint vec_len;
};

uniform sampler2DArray data;

// GLOBAL VARIABLES
// ================
vec3  tex_size      = vec3(textureSize(data, 0)) - 1;
vec2  one_zero      = vec2(1.0, 0.0);
float step_size     = 1.0 / float(iterations_per_step);
int   pixel_per_vec = int(max(ceil(float(vec_len) / float(channels)), 1));
int   max_peak_id   = 2 * int(tex_size.z) + 1;

/*
// Semimanual trilinear interpolation
vec4 velo_from_texture(sampler2DArray tex, vec3 P0)
{
  // Same point on next depth-level (if depth is fractional)
  vec3 P1 = vec3(P0.xy, ceil(P0.z));

  // Denormalization Parameter
  // x_range: max - min
  float c_min   = peaks[2 * int(P0.z)];
  float c_range = peaks[2 * int(P0.z) + 1] - c_min;
  float d_min   = peaks[2 * int(P1.z)];
  float d_range = peaks[2 * int(P1.z) + 1] - d_min;

  // Automatic bilinear interpolation + denormalization
  vec4 v0 = texture(data, P0) * c_range + c_min;
  vec4 v1 = texture(data, P1) * d_range + d_min;

  // Manual trilinear interpolation
  float lambda = fract(P0.z);
  return mix(v0, v1, lambda);
}
*/

// Manual trilinear interpolation (most precise)
//     c3--------c2           d3--------d2
//      | s       | lambda     | s       |
//      |---c     |----P       |---d     |
//      |   | t   |            |   | t   |
//     c0--------c1           d0--------d1
//     c0 := P0               d2 := P1
vec4 velo_from_texel(sampler2DArray tex, vec3 P)
{
  // Reference points to resemble all interpolated texels
  ivec4 P0 =  ivec4(floor(P), 1)                   * ivec4(pixel_per_vec, 1, 1, 1);
  ivec4 P1 = (ivec4(floor(P), 1) + ivec4(1,1,1,0)) * ivec4(pixel_per_vec, 1, 1, 1);

  // Factors for trilinear interpolation
  float s      = fract(P.x);
  float t      = fract(P.y);
  float lambda = fract(P.z);

  // Denormalization Parameter
  // x_range: max - min
  int   n_id    = 2 * P0.z;
  float c_min   = peaks[min(n_id    , max_peak_id)];
  float c_range = peaks[min(n_id + 1, max_peak_id)] - c_min;
  float d_min   = peaks[min(n_id + 2, max_peak_id)];
  float d_range = peaks[min(n_id + 3, max_peak_id)] - d_min;

  vec4 velocity = vec4(0,0,0,0);
  for (int p = 0; p < pixel_per_vec; p++)
  {
    // Lower depth
    vec4 c0 = texelFetch(tex, P0.xyz,          0) * c_range + c_min;
    vec4 c1 = texelFetch(tex, P0.wyz + P1.xww, 0) * c_range + c_min;
    vec4 c2 = texelFetch(tex, P0.wwz + P1.xyw, 0) * c_range + c_min;
    vec4 c3 = texelFetch(tex, P0.xwz + P1.wyw, 0) * c_range + c_min;

    // Higher depth
    vec4 d0 = texelFetch(tex, P0.xyw + P1.wwz, 0) * d_range + d_min;
    vec4 d1 = texelFetch(tex, P0.wyw + P1.xwz, 0) * d_range + d_min;
    vec4 d2 = texelFetch(tex,          P1.xyz, 0) * d_range + d_min;
    vec4 d3 = texelFetch(tex, P0.xww + P1.wyz, 0) * d_range + d_min;

    // Bilinear interpolation (2D, near)
    vec4 i0 = mix(c0, c1, s);
    vec4 i1 = mix(c3, c2, s);
    vec4 c  = mix(i0, i1, t);

    // Bilinear interpolation (2D, far)
    vec4 j0 = mix(d0, d1, s);
    vec4 j1 = mix(d3, d2, s);
    vec4 d  = mix(j0, j1, t);
    
    // Trilinear interpolation (3D)
    for (uint ch = 0; (ch < channels) && (ch < vec_len); ch++)
    {
      uint component       = p * channels + ch;
      velocity[component] = mix(c[ch], d[ch], lambda);
    }

    // To address the next vector components   
    P0 += ivec4(1, 0, 0, 0);
    P1 += ivec4(1, 0, 0, 0);
  }
  // Trilinear interpolation (3D)
  return velocity;
}


// Improved Euler Scheme
// Manual trilinear interpolation using the real position is significantly more precise,
// since normalization for texture(...)-access introduces huge rounding errors.
vec4 update_velocity(vec4 l_pos)
{
  // Left slope
  vec4 l_velo  = velo_from_texel(data, abs(l_pos.xyz));

  // Right slope
  vec4 r_pos   = l_pos + step_size * l_velo;
  vec4 r_velo  = velo_from_texel(data, abs(r_pos.xyz));
  
  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}


vec4 update_position(vec4 pos, vec4 velo)
{
  return pos + step_size * velo;
}


// MAIN
//=====
void main()
{
  uint id = uint(gl_GlobalInvocationID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x) * steps * iterations_per_step;

  vec4 pos  = vec4(gl_GlobalInvocationID.xyz * seeding_stride.xyz + seeding_offset0.xyz, 1);
  vec4 velo = vec4(0,0,0,0);

  float c_min   = peaks[0];
  float c_range = peaks[1] - c_min;

  for (int i = 0; i < steps * iterations_per_step; i++)
  {
    pos  = update_position(pos, velo);
    velo = update_velocity(pos);
    positions[id + i]  = pos;
    velocities[id + i] = velo;
  }
  
}