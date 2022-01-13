// Common
// ======
// 1/4

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

layout(std430, binding = 2) buffer DenormalizationBuffer
{
  float peaks[];
};

layout(std430, binding = 3) buffer TestBuffer
{
  float test[];
};

layout(std140, binding = 0) uniform SeedingBuffer
{
  vec4  seeding_stride;
  uvec2 seeding_range_x;
  uvec2 seeding_range_y;
  uvec2 seeding_range_z;
};

layout(std140, binding = 1) uniform IntegrationBuffer
{
  uvec4 integration_grid;
  vec4  integration_cell_size;
  float integration_stepsize_h;
  float integration_stepsize_dt;
  float integration_ds_factor;
  uint  integration_global_stepcount;
  uint  integration_local_stepcount;
  uint  integration_remainder_stepcount;
  uint  integration_strategy;
  uint  integration_texelfetch;
};

// Uniforms
uniform uint global_time;
uniform int fin;

// Global Variables
vec2  one_zero  = vec2(1.0, 0.0);
vec3 tex_size;

bool check_boundaries(vec4 pos)
{
  if ((pos.x < 0) || (pos.y < 0) || (pos.z < 0))
    return false;

  if ((pos.x > tex_size.x) || (pos.y > tex_size.y) || (pos.z > tex_size.z))
    return false;
  
  return true;
}

bool check_depth(float d)
{
  if (d < 0)
    return false;

  if (d > tex_size.z)
    return false;
  
  return true;
}

// Denormalization functions
vec4 denormalize_depth_1N(vec4 texel, int depth, bool using_data2)
{
  if (!check_depth(depth))
    depth = max(min(depth, int(tex_size.z) * 2), 0);

  int id = depth + int(global_time * integration_grid.z);
  if (using_data2) // next timelayer
  {
    id += int(integration_grid.z);
  }
  id *= 2;

  //vec2  min_max = peaks[id];
  vec2  min_max = vec2(peaks[id], peaks[id+1]);
  return vec4(texel) * (min_max[1] - min_max[0]) + min_max[0];
}

vec4 denormalize_depth_1N(vec4 texel, float depth, bool using_data2)
{
  if (!check_depth(depth))
    depth = max(min(depth, int(tex_size.z) * 2), 0);

  int id = int(depth) + int(global_time * integration_grid.z);
  if (using_data2) // next timelayer
  {
    id += int(integration_grid.z);
  }
  id *= 2;

  //vec2  min_max = peaks[id];
  vec2  min_max = vec2(peaks[id], peaks[id+1]);
  return vec4(texel) * (min_max[1] - min_max[0]) + min_max[0];
}

vec4 denormalize_depth_3N(vec4 texel, int depth, bool using_data2)
{
  if (!check_depth(depth))
    depth = max(min(depth, int(tex_size.z) * 2 * 3), 0);

  int id = depth;
  if (using_data2) // next timelayer
  {
    id += int((global_time + 1) * integration_grid.z);
  }
  else
  {
    id += int((global_time) * integration_grid.z);
  }
  id *= 2 * 3; //2: min/max, 3: vec_len

  vec2 min_max_x = vec2(peaks[id], peaks[id+1]);
  vec2 min_max_y = vec2(peaks[id+2], peaks[id+3]);
  vec2 min_max_z = vec2(peaks[id+4], peaks[id+5]);

  vec4 range, minimum;
  range.x = min_max_x[1] - min_max_x[0];
  range.y = min_max_y[1] - min_max_y[0];
  range.z = min_max_z[1] - min_max_z[0];
  range.w = 0.0;
  minimum.x = min_max_x[0];
  minimum.y = min_max_y[0];
  minimum.z = min_max_z[0];
  minimum.w = 0.0;

  return vec4(texel) * range + minimum;
}

vec4 denormalize_depth_3N(vec4 texel, float depth, bool using_data2)
{
  if (!check_depth(depth))
    depth = max(min(depth, int(tex_size.z) * 2 * 3), 0);

  int id = int(depth);
  if (using_data2) // next timelayer
  {
    id += int((global_time + 1) * integration_grid.z);
  }
  else
  {
    id += int((global_time) * integration_grid.z);
  }
  id *= 2 * 3; //2: min/max, 3: vec_len

  vec2 min_max_x = vec2(peaks[id], peaks[id+1]);
  vec2 min_max_y = vec2(peaks[id+2], peaks[id+3]);
  vec2 min_max_z = vec2(peaks[id+4], peaks[id+5]);

  vec4 range, minimum;
  range.x = min_max_x[1] - min_max_x[0];
  range.y = min_max_y[1] - min_max_y[0];
  range.z = min_max_z[1] - min_max_z[0];
  range.w = 0.0;
  minimum.x = min_max_x[0];
  minimum.y = min_max_y[0];
  minimum.z = min_max_z[0];
  minimum.w = 0.0;

  return vec4(texel) * range + minimum;
}
