
// 2DArray Sampler Body
// ================================
// 3/4

// TEXTURE
// =======
// Improved Euler Scheme
vec4 update_velocity_euler_texture(sampler2DArray tex, vec4 l_pos, vec4 stepsize, bool using_data2)
{
  // Left slope
  vec4 l_velo  = texture_velo(tex, l_pos, using_data2);

  // Right slope
  vec4 r_pos   = l_pos + stepsize * l_velo;
  vec4 r_velo  = texture_velo(tex, r_pos, using_data2);

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

// Unsteady Euler
vec4 update_velocity_euler_texture_refined(sampler2DArray tex0, sampler2DArray tex1, vec4 pos, float h, float rel_dt, float curr_time, float vector_factor, vec4 cell_factor)
{
  // Left slope
  vec4 l_velo_t0 = texture_velo(tex0, pos, false);
  vec4 l_velo_t1 = texture_velo(tex1, pos, true);
  vec4 l_velo = mix(l_velo_t0, l_velo_t1, curr_time) * vector_factor * cell_factor;

  // Right slope
  vec4 r_velo_t0 = texture_velo(tex0, pos + h * l_velo, false);
  vec4 r_velo_t1 = texture_velo(tex1, pos + h * l_velo, true);
  vec4 r_velo = mix(l_velo_t0, l_velo_t1, curr_time + rel_dt) * vector_factor * cell_factor;

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

// Steady Euler
vec4 update_velocity_euler_texture_refined(sampler2DArray tex, vec4 pos, float h, float vector_factor, vec4 cell_factor)
{
  // Left slope
  vec4 l_velo  = texture_velo(tex, pos, false) * vector_factor * cell_factor;

  // Right slope
  vec4 r_velo  = texture_velo(tex, pos + h * l_velo, false) * vector_factor * cell_factor;

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

// Runge Kutta 4th order
vec4 update_velocity_rk4_texture(sampler2DArray tex, vec4 y0, vec4 stepsize, bool using_data2)
{
  vec4  k1 = texture_velo(tex, y0, using_data2);
  vec4  y1 = y0 + stepsize * (k1/2.0);

  vec4  k2 = texture_velo(tex, y1, using_data2);
  vec4  y2 = y0 + stepsize * (k2/2.0);

  vec4  k3 = texture_velo(tex, y2, using_data2);
  vec4  y3 = y0 + stepsize *  k3;

  vec4  k4 = texture_velo(tex, y3, using_data2);

  // Using Euler method additionally:
  /*
  vec4  k1 = update_velocity_euler_texture(tex, y0, stepsize, using_data2);
  vec4  y1 = y0 + stepsize * (k1/2.0);

  vec4  k2 = update_velocity_euler_texture(tex, y1, stepsize, using_data2);
  vec4  y2 = y0 + stepsize * (k2/2.0);

  vec4  k3 = update_velocity_euler_texture(tex, y2, stepsize, using_data2);
  vec4  y3 = y0 + stepsize *  k3;

  vec4  k4 = update_velocity_euler_texture(tex, y3, stepsize, using_data2);
  */

  vec4  velo_rk4 = (k1 + 2*k2 + 2*k3 + k4) / 6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Unsteady RK4
vec4 update_velocity_rk4_texture_refined(sampler2DArray tex0, sampler2DArray tex1, vec4 pos, float h, float rel_dt, float curr_time, float vector_factor, vec4 cell_factor)
{
  vec4 k1_t0 = texture_velo(tex0, pos, false);
  vec4 k1_t1 = texture_velo(tex1, pos, true);
  vec4 k1 = mix(k1_t0, k1_t1, curr_time) * vector_factor * cell_factor;

  vec4 k2_t0 = texture_velo(tex0, pos + h * k1/2.0, false);
  vec4 k2_t1 = texture_velo(tex1, pos + h * k1/2.0, true);
  vec4 k2 = mix(k2_t0, k2_t1, (curr_time + 0.5 * rel_dt)) * vector_factor * cell_factor;

  vec4 k3_t0 = texture_velo(tex0, pos + h * k2/2.0, false);
  vec4 k3_t1 = texture_velo(tex1, pos + h * k2/2.0, true);
  vec4 k3 = mix(k3_t0, k3_t1, (curr_time + 0.5 * rel_dt)) * vector_factor * cell_factor;

  vec4 k4_t0 = texture_velo(tex0, pos + h * k3, false);
  vec4 k4_t1 = texture_velo(tex1, pos + h * k3, true);
  vec4 k4 = mix(k4_t0, k4_t1, (curr_time + 1 * rel_dt)) * vector_factor * cell_factor;

  vec4 velo_rk4 = (k1 + 2*k2 + 2*k3 + k4) / 6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Steady RK4
vec4 update_velocity_rk4_texture_refined(sampler2DArray tex, vec4 y0, float h, float vector_factor, vec4 cell_factor)
{
  vec4 k1 = texture_velo(tex, y0, false) * vector_factor * cell_factor;
  vec4 k2 = texture_velo(tex, y0 + h * k1/2.0, false) * vector_factor * cell_factor;
  vec4 k3 = texture_velo(tex, y0 + h * k2/2.0, false) * vector_factor * cell_factor;
  vec4 k4 = texture_velo(tex, y0 + h * k3, false) * vector_factor * cell_factor;

  vec4 velo_rk4 = (k1 + 2*k2 + 2*k3 + k4) / 6.0;

  return velo_rk4 * one_zero.xxxy;
}

// TEXELFETCH
// ==========
// Improved Euler Scheme
vec4 update_velocity_euler_texel(sampler2DArray tex, vec4 l_pos, vec4 stepsize, bool using_data2)
{
  // Left slope
  vec4 l_velo  = texelfetch_velo(tex, l_pos, using_data2);

  // Right slope
  vec4 r_pos   = l_pos + stepsize * l_velo;
  vec4 r_velo  = texelfetch_velo(tex, r_pos, using_data2);

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

// Unsteady Euler
vec4 update_velocity_euler_texel_refined(sampler2DArray tex0, sampler2DArray tex1, vec4 pos, float h, float rel_dt, float curr_time, float vector_factor, vec4 cell_factor)
{
  // Left slope
  vec4 l_velo_t0 = texelfetch_velo(tex0, pos, false);
  vec4 l_velo_t1 = texelfetch_velo(tex1, pos, true);
  vec4 l_velo = mix(l_velo_t0, l_velo_t1, curr_time) * vector_factor * cell_factor;

  // Right slope
  vec4 r_velo_t0 = texelfetch_velo(tex0, pos + h * l_velo, false);
  vec4 r_velo_t1 = texelfetch_velo(tex1, pos + h * l_velo, true);
  vec4 r_velo = mix(l_velo_t0, l_velo_t1, curr_time + rel_dt) * vector_factor * cell_factor;

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}

// Steady Euler
vec4 update_velocity_euler_texel_refined(sampler2DArray tex, vec4 pos, float h, float vector_factor, vec4 cell_factor)
{
  // Left slope
  vec4 l_velo  = texelfetch_velo(tex, pos, false) * vector_factor * cell_factor;

  // Right slope
  vec4 r_velo  = texelfetch_velo(tex, pos + h * l_velo, false) * vector_factor * cell_factor;

  // Average
  return mix(l_velo, r_velo, 0.5) * one_zero.xxxy;
}


// Runge Kutta 4th order
vec4 update_velocity_rk4_texel(sampler2DArray tex, vec4 y0, vec4 stepsize, bool using_data2)
{
  vec4  k1 = texelfetch_velo(tex, y0, using_data2);
  vec4  y1 = y0 + stepsize * (k1/2.0);

  vec4  k2 = texelfetch_velo(tex, y1, using_data2);
  vec4  y2 = y0 + stepsize * (k2/2.0);

  vec4  k3 = texelfetch_velo(tex, y2, using_data2);
  vec4  y3 = y0 + stepsize *  k3;

  vec4  k4 = texelfetch_velo(tex, y3, using_data2);

  // Using the Euler method additionally:
  /*
  vec4  k1 = update_velocity_euler_texel(tex, y0, stepsize, using_data2);
  vec4  y1 = y0 + stepsize * (k1/2.0);

  vec4  k2 = update_velocity_euler_texel(tex, y1, stepsize, using_data2);
  vec4  y2 = y0 + stepsize * (k2/2.0);

  vec4  k3 = update_velocity_euler_texel(tex, y2, stepsize, using_data2);
  vec4  y3 = y0 + stepsize *  k3;

  vec4  k4 = update_velocity_euler_texel(tex, y3, stepsize, using_data2);
  */
  

  vec4  velo_rk4 = k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Unsteady RK4
vec4 update_velocity_rk4_texel_refined(sampler2DArray tex0, sampler2DArray tex1, vec4 pos, float h, float rel_dt, float curr_time, float vector_factor, vec4 cell_factor)
{
  vec4 k1_t0 = texelfetch_velo(tex0, pos, false);
  vec4 k1_t1 = texelfetch_velo(tex1, pos, true);
  vec4 k1 = mix(k1_t0, k1_t1, curr_time) * vector_factor * cell_factor;

  vec4 k2_t0 = texelfetch_velo(tex0, pos + h * k1/2.0, false);
  vec4 k2_t1 = texelfetch_velo(tex1, pos + h * k1/2.0, true);
  vec4 k2 = mix(k2_t0, k2_t1, (curr_time + 0.5 * rel_dt)) * vector_factor * cell_factor;

  vec4 k3_t0 = texelfetch_velo(tex0, pos + h * k2/2.0, false);
  vec4 k3_t1 = texelfetch_velo(tex1, pos + h * k2/2.0, true);
  vec4 k3 = mix(k3_t0, k3_t1, (curr_time + 0.5 * rel_dt)) * vector_factor * cell_factor;

  vec4 k4_t0 = texelfetch_velo(tex0, pos + h * k3, false);
  vec4 k4_t1 = texelfetch_velo(tex1, pos + h * k3, true);
  vec4 k4 = mix(k4_t0, k4_t1, (curr_time + 1 * rel_dt)) * vector_factor * cell_factor;

  vec4 velo_rk4 = (k1 + 2*k2 + 2*k3 + k4) / 6.0;

  return velo_rk4 * one_zero.xxxy;
}

// Steady RK4
vec4 update_velocity_rk4_texel_refined(sampler2DArray tex, vec4 pos, float h, float vector_factor, vec4 cell_factor)
{
  vec4 k1 = texelfetch_velo(tex, pos, false) * vector_factor * cell_factor;
  vec4 k2 = texelfetch_velo(tex, pos + h * k1/2.0, false) * vector_factor * cell_factor;
  vec4 k3 = texelfetch_velo(tex, pos + h * k2/2.0, false) * vector_factor * cell_factor;
  vec4 k4 = texelfetch_velo(tex, pos + h * k3, false) * vector_factor * cell_factor;

  vec4 velo_rk4 = (k1 + 2*k2 + 2*k3 + k4) / 6.0;

  return velo_rk4 * one_zero.xxxy;
} 
