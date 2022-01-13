
// Main (unsteady advection)
// =========================
// 4/4

vec4 update_position(vec4 pos, vec4 velo, float h)
{
  return (pos + h * velo);
}

void advect_euler_texture(vec4 pos, vec4 velo, float h, float rel_dt, float vector_factor, vec4 rel_cell_size, int id, int id_offset, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);

  // Interpolation factor of the 2 present textures
  float local_time = 0.0;

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);

      if (fin == 1)
        velo = update_velocity_euler_texture_refined(data1, pos, h, vector_factor, cell_factor);
      else
        velo = update_velocity_euler_texture_refined(data1, data2, pos, h, rel_dt, local_time, vector_factor, cell_factor);
    }
    positions[id + id_offset + i] = pos;
    velocities[id + id_offset + i] = velo;

    local_time += rel_dt;
  }
}

void advect_rk4_texture(vec4 pos, vec4 velo, float h, float rel_dt, float vector_factor, vec4 rel_cell_size, int id, int id_offset, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);

  // Interpolation factor of the 2 present textures
  float local_time = 0.0;

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);

      if (fin == 1)
        velo = update_velocity_rk4_texture_refined(data1, pos, h, vector_factor, cell_factor);
      else
        velo = update_velocity_rk4_texture_refined(data1, data2, pos, h, rel_dt, local_time, vector_factor, cell_factor);
    }

    positions[id + id_offset + i] = pos;
    velocities[id + id_offset + i] = velo;

    local_time += rel_dt;
  }
}

void advect_euler_texelfetch(vec4 pos, vec4 velo, float h, float rel_dt, float vector_factor, vec4 rel_cell_size, int id, int id_offset, uint steps)
{  
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);

  // Interpolation factor of the 2 present textures
  float local_time = 0.0;

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);

      if (fin == 1)
        velo = update_velocity_euler_texel_refined(data1, pos, h, vector_factor, cell_factor);
      else
        velo = update_velocity_euler_texel_refined(data1, data2, pos, h, rel_dt, local_time, vector_factor, cell_factor);
    }

    positions[id + id_offset + i] = pos;
    velocities[id + id_offset + i] = velo;

    local_time += rel_dt;
  }
}

void advect_rk4_texelfetch(vec4 pos, vec4 velo, float h, float rel_dt, float vector_factor, vec4 rel_cell_size, int id, int id_offset, uint steps)
{
  vec4 cell_factor = vec4(rel_cell_size.xyz, 1);

  // Interpolation factor of the 2 present textures
  float local_time = 0.0;

  for (int i = 0; i < steps; i++)
  {
    if (check_boundaries(pos))
    {
      pos  = update_position(pos, velo, h);

      if (fin == 1)
        velo = update_velocity_rk4_texel_refined(data1, pos, h, vector_factor, cell_factor);
      else
        velo = update_velocity_rk4_texel_refined(data1, data2, pos, h, rel_dt, local_time, vector_factor, cell_factor);
    }

    positions[id + id_offset + i] = pos;
    velocities[id + id_offset + i] = velo;

    local_time += rel_dt;
  }
}

void main()
{
  // Initial position of current seed in Buffer
  int id = int(gl_GlobalInvocationID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x) * int(integration_global_stepcount);
  // Track of current seeds position in Buffer
  int id_offs = int(global_time * integration_local_stepcount);

  tex_size = vec3(textureSize(data1, 0) - ivec3(1, 1, 1));

  // Velocity Vector factor
  float cell_min = min(min(integration_cell_size.x, integration_cell_size.y), integration_cell_size.z);
  vec4 cell_factor = vec4(cell_min / integration_cell_size.xyz, 1.0);

  // Last known position / velocity
  vec4 pos = positions[id + id_offs - 1];
  vec4 velo = velocities[id + id_offs - 1];

  // If no prior position / velocity
  if (global_time == 0)
  {
    uvec3 seeding_offset = uvec3(seeding_range_x.x, seeding_range_y.x, seeding_range_z.x);
    pos   = vec4(gl_GlobalInvocationID.xyz * seeding_stride.xyz + seeding_offset, 1);
    velo  = vec4(0,0,0,0);
  }

  float h = integration_stepsize_h;
  float dt = integration_stepsize_dt;
  float rel_dt = 1.0 / integration_local_stepcount;   // To increment local_time properly up to 1.0 regardless of cells timeslice size
  float vector_factor = integration_ds_factor;
  
  // There are 4 different integration strategies:
  // 1. Using texture(..) method & improved Euler integration
  // 2. Using texture(..) method & Runge Kutta 4th Order integration
  // 3. Using texelFetch(..) method & improved Euler integration
  // 4. Using texelFetch(..) method & Runge Kutta 4th Order integration
  // - in all cases the velocity of the current and the next timeslice will be interpolated manually
  // - in case of an 2D Texture Array the interpolation in z-dimension will be applied manually
  // - in case of the texelFetch(..) method the interpolation in all directions will be applied manually

  if (fin == 0)
  {
    if (integration_texelfetch == 0)
      if (integration_strategy == 0)
        advect_euler_texture(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_local_stepcount);
      else
        advect_rk4_texture(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_local_stepcount);

    else

        if (integration_strategy == 0)
          advect_euler_texelfetch(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_local_stepcount);
        else
          advect_rk4_texelfetch(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_local_stepcount);
  }
  
  // On last advection do the remainder steps
  if (fin == 1)
  {
    pos = positions[id + id_offs - 1];
    velo = velocities[id + id_offs - 1];

    if (integration_texelfetch == 0)
        if (integration_strategy == 0)
          advect_euler_texture(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_remainder_stepcount);
        else
          advect_rk4_texture(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_remainder_stepcount);

    else

        if (integration_strategy == 0)
          advect_euler_texelfetch(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_remainder_stepcount);
        else
          advect_rk4_texelfetch(pos, velo, h, rel_dt, vector_factor, cell_factor, id, id_offs, integration_remainder_stepcount);
  }
}
