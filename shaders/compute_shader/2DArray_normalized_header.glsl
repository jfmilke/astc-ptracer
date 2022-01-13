
// 2DArray Sampler Header (normalized)
// ===================================
// 2/4

uniform sampler2DArray data1;
uniform sampler2DArray data2;

// Manual trilinear interpolation (most precise, but slow)
//     c3--------c2           d3--------d2
//      | s       | lambda     | s       |
//      |---c     |----P       |---d     |
//      |   | t   |            |   | t   |
//     c0--------c1           d0--------d1
//     P0 := c0               P1 := d2
//
vec4 texelfetch_velo(sampler2DArray tex, vec4 pos, bool using_data2)
{
  // Reference points (4th component is zeroed for easy swizzling)
  ivec4 P0 = ivec4(floor(pos.xyz), 0);
  ivec4 P1 = ivec4(floor(pos.xyz), 0) + ivec4(1,1,1,0);

  // Factors for trilinear interpolation
  float s      = fract(pos.x);
  float t      = fract(pos.y);
  float lambda = fract(pos.z);

  // Lower Depth
  vec4 c0 = denormalize_depth(texelFetch(tex, P0.xyz,          0), P0.z         , using_data2);
  vec4 c1 = denormalize_depth(texelFetch(tex, P0.wyz + P1.xww, 0), P0.z, using_data2);
  vec4 c2 = denormalize_depth(texelFetch(tex, P0.wwz + P1.xyw, 0), P0.z, using_data2);
  vec4 c3 = denormalize_depth(texelFetch(tex, P0.xwz + P1.wyw, 0), P0.z, using_data2);

  // Higher depth
  vec4 d0 = denormalize_depth(texelFetch(tex, P0.xyw + P1.wwz, 0), P1.z, using_data2);
  vec4 d1 = denormalize_depth(texelFetch(tex, P0.wyw + P1.xwz, 0), P1.z, using_data2);
  vec4 d2 = denormalize_depth(texelFetch(tex,          P1.xyz, 0), P1.z, using_data2);
  vec4 d3 = denormalize_depth(texelFetch(tex, P0.xww + P1.wyz, 0), P1.z, using_data2);

  // Bilinear interpolation (2D, near)
  vec4 i0 = mix(c0, c1, s);
  vec4 i1 = mix(c3, c2, s);
  vec4 c  = mix(i0, i1, t);

  // Bilinear interpolation (2D, far)
  vec4 j0 = mix(d0, d1, s);
  vec4 j1 = mix(d3, d2, s);
  vec4 d  = mix(j0, j1, t);

  // Trilinear interpolation (3D)
  // 4th component will be zeroed later
  return mix(c, d, lambda);
}

// For 2D Texture Arrays the depth-component must be interpolated manually (fractional depth is not allowed here).
//       fract(pos.z)
// [near]----P    [far]
//
vec4 texture_velo(sampler2DArray tex, vec4 pos, bool using_data2)
{
  vec3 near = (pos.xyz * one_zero.xxy / tex_size) + floor(pos.z) * one_zero.yyx;
  vec3 far = (pos.xyz * one_zero.xxy / tex_size) + ceil(pos.z) * one_zero.yyx;
  
  vec4 c = denormalize_depth(texture(tex, near), near.z, using_data2);
  vec4 d = denormalize_depth(texture(tex, far), far.z, using_data2);

  // 4th component will be zeroed later
  return mix(c, d, fract(pos.z));
}
