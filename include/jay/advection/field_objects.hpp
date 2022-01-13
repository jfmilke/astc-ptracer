#ifndef JAY_ADVECTION_FIELD_OBJECTS_HPP
#define JAY_ADVECTION_FIELD_OBJECTS_HPP

#include <glbinding/gl/types.h>
#include <glbinding/gl/enum.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <jay/export.hpp>

namespace jay
{
  struct compute_conf
  {
    bool prefer_arraytexture;
    bool astc_compressed;

    int pos_binding;
    int vel_binding;
    int denorm_binding;
  };

  struct render_conf
  {
    std::string vs_code;
    std::string fs_code;
    std::string gs_code;

    std::size_t seed_count;
    std::size_t step_count;

    int pos_index;
    int vel_index;
  };

  struct shading_conf
  {
    gl::GLfloat min_velo_coloring;
    gl::GLfloat max_velo_coloring;
    gl::GLfloat damping;
  };

  struct texture_conf
  {
    gl::GLenum target;
    gl::GLint level;
    gl::GLenum internal_format;
    glm::vec3 size;
    gl::GLint border;
    gl::GLenum format;
    gl::GLenum type;
    gl::GLenum min_filter;
    gl::GLenum mag_filter;
    gl::GLenum wrap_s;
    gl::GLenum wrap_t;
    gl::GLenum wrap_r;
    
    std::size_t compressed_byte_size;

    std::vector<std::string> sampler_names;
    std::vector<int> indices;
  };

  struct seeding_conf
  {
    // UBO Content
    glm::vec3 stride;
    glm::uvec2 range_x;
    glm::uvec2 range_y;
    glm::uvec2 range_z;

    // Meta info
    glm::vec3 seeds;
    int binding;
    std::string name;
  };

  struct integration_conf
  {
    // UBO Content (Strategies)
    gl::GLuint strategy;
    gl::GLuint texelfetch;
    // UBO Content (continued)
    glm::uvec4 grid;
    glm::vec4 cell_size;
    gl::GLfloat step_size_h;
    gl::GLfloat step_size_dt;
    gl::GLfloat dataset_factor;

    gl::GLuint global_step_count;
    gl::GLuint local_step_count;
    gl::GLuint remainder_step_count;

    // Meta info
    int binding;
    std::string name;
  };

  struct draw_conf
  {
    float draw_percent;
    std::uint32_t draw_seeds;
    std::uint32_t draw_offset;
    glm::vec4 base_color;
  };

  struct unsteady_timings
  {
    double texture_uploads    = 0.0;
    double advections         = 0.0;
    double texture_upload_two = 0.0;
    double advection_single   = 0.0;
    std::size_t iterations    = 0;
  };
}

#endif