#version 450

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 velocity;

layout(location = 0) out vec4 color;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 normal;
layout(location = 3) out float strength;


uniform float min_velo_threshold;
uniform float max_velo_threshold;
uniform float damping;
uniform mat4 View;
uniform mat4 Proj;
uniform vec4 base_color;
uniform vec3 eye_pos;

vec3 lightPos = eye_pos;

// All components are in the range [0…1], including hue.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 ambientLighting(float strength)
{
  return vec3(strength, strength, strength);
}

vec3 diffuseLighting(vec3 normal)
{
  vec3 lightDir = normalize(lightPos - vertex.xyz);
  float diff = max(dot(normal, lightDir), 0.0);

  return vec3(diff, diff, diff);
}

void main()
{
  gl_Position = Proj * View * vertex;
  fragPos = vertex.xyz;
  //normal = normalize(cross(velocity.xyz, vec3(0,1,0)) + cross(velocity.xyz, vec3(1,0,0)));
  
  strength = length(velocity);

  vec3 base_color = vec3(0,0,1);

  if (true)
  {
    vec4 direction = normalize(velocity);
    float divergion = acos(dot(direction.xyz, vec3(1,0,0)));//+ acos(dot(direction.xyz, vec3(1,0,0)));

    vec3 hsv = rgb2hsv(base_color.xyz);
    // Color
    hsv.x;
    // Saturation
    hsv.y = strength / max_velo_threshold;
    // Brightness
    hsv.z = divergion;

    color.xyz = hsv2rgb(hsv); 
    // color.xyz *= (ambientLighting(0.3) + diffuseLighting(normal));
    color.w = 1.0;
  }
  else if (true)
  {
    vec3 hsv = rgb2hsv(base_color.xyz);
    // Color
    hsv.x;
    // Saturation
    hsv.y;
    // Brightness
    hsv.z = strength / max_velo_threshold;

    color.xyz = hsv2rgb(hsv);
    //color.xyz *= (ambientLighting(0.3) + diffuseLighting(vec3(1,1,1)));
    color.w = 1.0;
  }
  else
  {
    vec4 direction = normalize(velocity);

    color.xyz = direction.xyz;
    color.w = 1.0;
  }
}