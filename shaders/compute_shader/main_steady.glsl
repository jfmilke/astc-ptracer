
// Main (steady advection)
// =======================
// 4/4

vec4 update_position(vec4 pos, vec4 velo, float h)
{
  return (pos + h * velo);
}

void advect_euler_texture(vec4 pos, float h, float vector_factor, vec4 rel_cell_size, int id, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);
  vec4 velo = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);
      velo = update_velocity_euler_texture_refined(data1, pos, h, vector_factor, cell_factor);
    }
    positions[id + i] = pos;
    velocities[id + i] = velo;
  }
}

void advect_rk4_texture(vec4 pos, float h, float vector_factor, vec4 rel_cell_size, uint id, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);
  vec4 velo = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);
      velo = update_velocity_rk4_texture_refined(data1, pos, h, vector_factor, cell_factor);
    }
    positions[id + i] = pos;
    velocities[id + i] = velo;
  }
}

void advect_euler_texelfetch(vec4 pos, float h, float vector_factor, vec4 rel_cell_size, uint id, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);
  vec4 velo = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < integration_local_stepcount; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);
      velo = update_velocity_euler_texel_refined(data1, pos, h, vector_factor, cell_factor);
    }

    positions[id + i] = pos;
    velocities[id + i] = velo;
  }
}

void advect_rk4_texelfetch(vec4 pos, float h, float vector_factor, vec4 rel_cell_size, uint id, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);
  vec4 velo = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < integration_local_stepcount; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);
      velo = update_velocity_rk4_texel_refined(data1, pos, h, vector_factor, cell_factor);
    }

    positions[id + i] = pos;
    velocities[id + i] = velo;
  }
}

void main()
{
  // The ID resembles the position of the individual seed in the SSBO / Array Buffer.
  int id = int(gl_GlobalInvocationID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x) * int(integration_global_stepcount);

  tex_size = vec3(textureSize(data1, 0) - ivec3(1, 1, 1));

  float cell_min = min(min(integration_cell_size.x, integration_cell_size.y), integration_cell_size.z);
  vec4 cell_factor = vec4(cell_min / integration_cell_size.xyz, 1.0);

  // The initial seeds are placed evenly in the vectorfield (or a selection of it).
  uvec3 seeding_offset = uvec3(seeding_range_x.x, seeding_range_y.x, seeding_range_z.x);
  vec4 pos = vec4(gl_GlobalInvocationID.xyz * seeding_stride.xyz + seeding_offset, 1);

  float h = integration_stepsize_h;
  float vector_factor = integration_ds_factor;
  
  // There are 4 different integration strategies:
  // 1. Using texture(..) method & improved Euler integration
  // 2. Using texture(..) method & Runge Kutta 4th Order integration
  // 3. Using texelFetch(..) method & improved Euler integration
  // 4. Using texelFetch(..) method & Runge Kutta 4th Order integration
  // - in case of an 2D Texture Array the interpolation in z-dimension will be applied manually
  // - in case of the texelFetch(..) method the interpolation in all directions will be applied manually

  if (integration_texelfetch == 0)
    if (integration_strategy == 0)
      advect_euler_texture(pos, h, vector_factor, cell_factor, id, integration_local_stepcount);
    else
      advect_rk4_texture(pos, h, vector_factor, cell_factor, id, integration_local_stepcount);

  else

    if (integration_strategy == 0)
      advect_euler_texelfetch(pos, h, vector_factor, cell_factor, id, integration_local_stepcount);
    else
      advect_rk4_texelfetch(pos, h, vector_factor, cell_factor, id, integration_local_stepcount);
}
