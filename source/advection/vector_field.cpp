#include <jay/advection/vector_field.hpp>

#include <jay/io/data_io.hpp>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/bitfield.h>
#include <iostream>
#include <globjects/base/StringTemplate.h>

#include <jay/core/menu.hpp>


namespace jay
{
  vector_field::vector_field(bool steady_vectorfield)
    : output(std::make_unique<advected_field>())
    , p     (performance())
  {
    p.initialize_gl_queries(20);
    output->steady_advection = steady_vectorfield;
  }

  void vector_field::init_configuration(antMenu* menu, bool astc_compressed, bool texture_array)
  {
    if (t_conf == nullptr)
      init_texture_conf(menu, astc_compressed, texture_array);

    if (s_conf == nullptr)
      init_seeding_conf(menu);

    if (i_conf == nullptr)
      init_integration_conf(menu);

    if (c_conf == nullptr)
      init_compute_conf(menu, astc_compressed, texture_array);

    output->init_configuration(menu);
  }

  void vector_field::update_configuration(antMenu* menu)
  {
    update_seeding_conf(menu);
    update_integration_conf(menu);
  }

  void vector_field::init_texture_conf(antMenu* menu, bool astc_compressed, bool texture_array)
  {
    t_conf = new texture_conf();

    t_conf->target = (texture_array || astc_compressed) ? gl::GLenum::GL_TEXTURE_2D_ARRAY : gl::GLenum::GL_TEXTURE_3D;
    t_conf->level  = 0;
    t_conf->internal_format = (astc_compressed) ? get_internal_astc_format(menu->cmp_block_sizes) : get_internal_format(3);
    t_conf->size.x = (astc_compressed) ? menu->cmp_img_dimensions.x : menu->src_grid[0];
    t_conf->size.y = (astc_compressed) ? menu->cmp_img_dimensions.y : menu->src_grid[1];
    t_conf->size.z = menu->src_grid[2];
    t_conf->border = 0;
    t_conf->format = get_format(3);
    t_conf->type   = gl::GLenum::GL_FLOAT;

    t_conf->min_filter = gl::GLenum::GL_LINEAR;
    t_conf->mag_filter = gl::GLenum::GL_LINEAR;
    t_conf->wrap_s = gl::GLenum::GL_MIRRORED_REPEAT;
    t_conf->wrap_t = gl::GLenum::GL_MIRRORED_REPEAT;
    t_conf->wrap_r = gl::GLenum::GL_MIRRORED_REPEAT;

    t_conf->compressed_byte_size = menu->cmp_img_size * t_conf->size.z;

    t_conf->sampler_names.push_back("data1");
    t_conf->sampler_names.push_back("data2");
    t_conf->indices.push_back((int)gl::GLenum::GL_TEXTURE0);
    t_conf->indices.push_back((int)gl::GLenum::GL_TEXTURE1);
  }

  void vector_field::init_seeding_conf(antMenu* menu)
  {
    s_conf = new seeding_conf();

    s_conf->stride  = menu->seed_stride;
    s_conf->range_x = menu->seed_range_x;
    s_conf->range_y = menu->seed_range_y;
    s_conf->range_z = menu->seed_range_z;

    s_conf->seeds = menu->seed_directional;

    s_conf->binding = 0;
    s_conf->name = "SeedingBuffer";
  }

  void vector_field::init_integration_conf(antMenu* menu)
  {
    i_conf = new integration_conf();

    i_conf->strategy = (gl::GLuint) menu->int_strategy_rk4;
    i_conf->texelfetch = (gl::GLuint) menu->int_texelfetch;

    i_conf->grid.x = (menu->src_grid_dim >= 1) ? menu->src_grid[0] : 0;
    i_conf->grid.y = (menu->src_grid_dim >= 2) ? menu->src_grid[1] : 0;
    i_conf->grid.z = (menu->src_grid_dim >= 3) ? menu->src_grid[2] : 0;
    i_conf->grid.w = (menu->src_grid_dim >= 4) ? menu->src_grid[3] : 0;
    i_conf->cell_size = menu->int_cell_size;
    i_conf->step_size_h = menu->int_step_size_h;
    i_conf->step_size_dt = menu->int_step_size_dt;
    i_conf->dataset_factor = menu->int_dataset_factor;
    i_conf->global_step_count = menu->int_global_step_count;
    i_conf->local_step_count = menu->int_local_step_count;
    i_conf->remainder_step_count = menu->int_remainder_step_count;


    i_conf->binding = 1;
    i_conf->name = "IntegrationBuffer";
  }

  void vector_field::init_compute_conf(antMenu* menu, bool astc_compressed, bool texture_array)
  {
    c_conf = new compute_conf();

    c_conf->astc_compressed = astc_compressed;
    c_conf->denorm_binding = 2;
    c_conf->pos_binding = 0;
    c_conf->vel_binding = 1;
    c_conf->prefer_arraytexture = texture_array;
  }

  void vector_field::update_seeding_conf(antMenu* menu)
  {
    s_conf->stride = menu->seed_stride;
    s_conf->range_x = menu->seed_range_x;
    s_conf->range_y = menu->seed_range_y;
    s_conf->range_z = menu->seed_range_z;

    s_conf->seeds = menu->seed_directional;
  }

  void vector_field::update_integration_conf(antMenu* menu)
  {
    i_conf->strategy = (gl::GLuint) menu->int_strategy_rk4;
    i_conf->texelfetch = (gl::GLuint) menu->int_texelfetch;

    i_conf->cell_size = menu->int_cell_size;
    i_conf->step_size_h = menu->int_step_size_h;
    i_conf->step_size_dt = menu->int_step_size_dt;
    i_conf->dataset_factor = menu->int_dataset_factor;
    i_conf->global_step_count = menu->int_global_step_count;
    i_conf->local_step_count = menu->int_local_step_count;
    i_conf->remainder_step_count = menu->int_remainder_step_count;
  }

  double vector_field::setup_compute_shader(bool componentwise_normalized, bool measure_time)
  {
    // Root directory holding the following subpaths
    std::string shader_fp = "../shaders/compute_shader/";
    std::string shader_head = "";
    std::string shader_sampler = "";
    std::string shader_main = "";

    // At the moment only ASTC compressed data is normalized.
    // TODO: Generalize this.
    bool normalized = c_conf->astc_compressed;

    // 1. Common Stuff
    shader_head += data_io::read_shader_file(shader_fp + "common.glsl");

    // 2. Sampler
    if (t_conf->target == gl::GLenum::GL_TEXTURE_2D_ARRAY)
    {
      if (normalized)
        shader_sampler += data_io::read_shader_file(shader_fp + "2DArray_normalized_header.glsl");
      else
        shader_sampler += data_io::read_shader_file(shader_fp + "2DArray_regular_header.glsl");

      shader_sampler += data_io::read_shader_file(shader_fp + "2DArray_funcs.glsl");
    }

    if (t_conf->target == gl::GLenum::GL_TEXTURE_3D)
    {
      if (normalized)
        shader_sampler += data_io::read_shader_file(shader_fp + "3D_normalized_header.glsl");
      else
        shader_sampler += data_io::read_shader_file(shader_fp + "3D_regular_header.glsl");

      shader_sampler += data_io::read_shader_file(shader_fp + "3D_funcs.glsl");
    }

    // 3. Main function
    if (output->steady_advection)
      shader_sampler += data_io::read_shader_file(shader_fp + "main_steady.glsl");
    else
      shader_sampler += data_io::read_shader_file(shader_fp + "main_unsteady.glsl");

    // Assemble the shader code
    std::string shadercode = shader_head + shader_sampler + shader_main;


    compute_shader_source = globjects::Shader::sourceFromString(shadercode);
    if (componentwise_normalized)
      globjects::Shader::globalReplace("denormalize_depth_3N", "denormalize_depth");
    else
      globjects::Shader::globalReplace("denormalize_depth_1N", "denormalize_depth");
    compute_shader_template = globjects::Shader::applyGlobalReplacements(compute_shader_source.get());
    compute_shader = globjects::Shader::create(gl::GL_COMPUTE_SHADER, compute_shader_template.get());

    // You could print the code to console if you want
    std::cout << compute_shader->getSource() << std::endl;

    if (measure_time)
      p.issue_GPU_timestamp("Compute Shader Setup", generation_count);
      //p.start_measure_GPU_time(0);

    compute_program = globjects::Program::create();

    // Create & Link program
    compute_program->attach(compute_shader.get());
    compute_program->link();

    if (measure_time)
      p.issue_GPU_timestamp("Compute Shader Setup", generation_count);
      //return p.finish_measure_GPU_time(0) / 1000000.0; // ms

    return 0;
  }


  void vector_field::setup_texture(std::unique_ptr<globjects::Texture>& texture, int texture_index, std::string sampler_name)
  {
    texture = std::unique_ptr<globjects::Texture>(new globjects::Texture(t_conf->target));
    texture->bindActive(gl::GLenum::GL_TEXTURE0);
    texture->bind();
    texture->setParameter(gl::GLenum::GL_TEXTURE_MIN_FILTER, t_conf->min_filter);
    texture->setParameter(gl::GLenum::GL_TEXTURE_MAG_FILTER, t_conf->mag_filter);
    texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_S, t_conf->wrap_s);
    texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_T, t_conf->wrap_t);
    if (t_conf->target == gl::GLenum::GL_TEXTURE_3D)
      texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_R, t_conf->wrap_r);
    texture->image3D(t_conf->level, t_conf->internal_format, t_conf->size, t_conf->border, t_conf->format, t_conf->type, NULL);

    compute_program->setUniform(compute_program->getUniformLocation(sampler_name), texture_index);
  }

  double vector_field::setup_textures(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex Setup", generation_count);

    setup_texture(this->tex0_ptr, 0, t_conf->sampler_names[0]);

    if (!output->steady_advection)
      setup_texture(this->tex1_ptr, 1, t_conf->sampler_names[1]);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex Setup", generation_count);

    return 0.0;
  }

  void vector_field::setup_compressed_texture(std::unique_ptr<globjects::Texture>& texture, int texture_index, std::string sampler_name)
  {
    // Create the texture
    texture = std::unique_ptr<globjects::Texture>(new globjects::Texture(t_conf->target));
    texture->bindActive(gl::GLenum::GL_TEXTURE0);
    texture->bind();
    texture->setParameter(gl::GLenum::GL_TEXTURE_MIN_FILTER, t_conf->min_filter);
    texture->setParameter(gl::GLenum::GL_TEXTURE_MAG_FILTER, t_conf->mag_filter);
    texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_S, t_conf->wrap_s);
    texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_T, t_conf->wrap_t);
    if (t_conf->target == gl::GLenum::GL_TEXTURE_3D)
      texture->setParameter(gl::GLenum::GL_TEXTURE_WRAP_R, t_conf->wrap_r);
    texture->storage3D(0, t_conf->internal_format, t_conf->size);

    compute_program->setUniform(compute_program->getUniformLocation(sampler_name), texture_index);
  }

  double vector_field::setup_astc_textures(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex Setup (ASTC)", generation_count);

    setup_compressed_texture(this->tex0_ptr, 0, t_conf->sampler_names[0]);

    if (!output->steady_advection)
      setup_compressed_texture(this->tex1_ptr, 1, t_conf->sampler_names[1]);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex Setup (ASTC)", generation_count);

    return 0.0;
  }

  double vector_field::setup_storage_buffers(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("SSBO Setup", generation_count);


    auto seed_count = s_conf->seeds.x * s_conf->seeds.y * s_conf->seeds.z;
    auto vertex_count = i_conf->global_step_count * seed_count;

    // New values
    output->r_conf->seed_count = seed_count;
    output->r_conf->step_count = i_conf->global_step_count;

    // Reallocate storage
    output->b_positions = globjects::Buffer::create();
    output->b_positions->bindBase(gl::GL_SHADER_STORAGE_BUFFER, c_conf->pos_binding);
    output->b_positions->setData(vertex_count * sizeof(glm::vec4), NULL, gl::GL_DYNAMIC_DRAW);

    output->b_velocity = globjects::Buffer::create();
    output->b_velocity->bindBase(gl::GL_SHADER_STORAGE_BUFFER, c_conf->vel_binding);
    output->b_velocity->setData(vertex_count * sizeof(glm::vec4), NULL, gl::GL_DYNAMIC_DRAW);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("SSBO Setup", generation_count);

    return 0.0;
  }

  double vector_field::setup_uniform_buffers(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("UBO Setup", generation_count);

    ubo_seeding = std::unique_ptr<globjects::UniformBlock>(compute_program->uniformBlock(s_conf->name));
    b_seeding = std::unique_ptr<globjects::Buffer>(globjects::Buffer::create());
    b_seeding->bindBase(gl::GL_UNIFORM_BUFFER, s_conf->binding);
    b_seeding->setData(sizeof(glm::vec4) + 3 * sizeof(glm::uvec2), NULL, gl::GL_DYNAMIC_DRAW);

    ubo_integration = std::unique_ptr<globjects::UniformBlock>(compute_program->uniformBlock(i_conf->name));
    b_integration = std::unique_ptr<globjects::Buffer>(globjects::Buffer::create());
    b_integration->bindBase(gl::GL_UNIFORM_BUFFER, i_conf->binding);
    b_integration->setData(sizeof(glm::vec4) + sizeof(glm::uvec4) + 8 * 4, NULL, gl::GL_DYNAMIC_DRAW);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("UBO Setup", generation_count);

    return 0.0;
  }

  double vector_field::setup_denormalization_buffer(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Peaks SSBO Setup", generation_count);

    b_denormalization = globjects::Buffer::create();
    b_denormalization->bindBase(gl::GLenum::GL_SHADER_STORAGE_BUFFER, c_conf->denorm_binding);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Peaks SSBO Setup", generation_count);

    return 0.0;
  }

  void vector_field::setup_test_buffer()
  {
    b_test = globjects::Buffer::create();
    b_test->bindBase(gl::GLenum::GL_SHADER_STORAGE_BUFFER, 3);

    b_test->setData(500 * sizeof(float), NULL, gl::GL_DYNAMIC_DRAW);
  }


  double vector_field::update_denormalization_buffer(std::vector<float>& data, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Peaks SSBO Update", generation_count);

    b_denormalization->setData(data, gl::GLenum::GL_STATIC_DRAW);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Peaks SSBO Update", generation_count);

    return 0.0;
  }

  double vector_field::update_storage_buffers(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("SSBO Update", generation_count);

    auto seed_count = s_conf->seeds.x * s_conf->seeds.y * s_conf->seeds.z;
    auto vertex_count = i_conf->global_step_count * seed_count;

    output->r_conf->seed_count = seed_count;
    output->r_conf->step_count = i_conf->global_step_count;

    output->b_positions->bindBase(gl::GL_SHADER_STORAGE_BUFFER, c_conf->pos_binding);
    output->b_positions->setData(vertex_count * sizeof(glm::vec4), NULL, gl::GL_DYNAMIC_DRAW);

    output->b_velocity->bindBase(gl::GL_SHADER_STORAGE_BUFFER, c_conf->vel_binding);
    output->b_velocity->setData(vertex_count * sizeof(glm::vec4), NULL, gl::GL_DYNAMIC_DRAW);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("SSBO Update", generation_count);

    return 0.0;
  }

  void vector_field::update_texture(std::unique_ptr<globjects::Texture>& texture, const gl::GLvoid* data, std::size_t offset_byte)
  {
    texture->subImage3D(t_conf->level, glm::ivec3(0, 0, 0), t_conf->size, t_conf->format, t_conf->type, (std::uint8_t*) data + offset_byte);
  }

  double vector_field::update_texture(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex Update", generation_count);


    update_texture(tex0_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex Update", generation_count);

    return 0.0;
  }

  double vector_field::update_texture_t0(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex0 Update", generation_count);


    update_texture(tex0_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex0 Update", generation_count);

    return 0.0;
  }

  double vector_field::update_texture_t1(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex1 Update", generation_count);


    update_texture(tex1_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex1 Update", generation_count);

    return 0.0;
  }

  void vector_field::update_astc_texture(std::unique_ptr<globjects::Texture>& texture, const gl::GLvoid* data, std::size_t offset_byte)
  {
    texture->compressedImage3D(t_conf->level, t_conf->internal_format, t_conf->size, t_conf->border, t_conf->compressed_byte_size, (std::uint8_t*)(data) + offset_byte);
  }

  double vector_field::update_astc_texture(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex Update (ASTC)", generation_count);


    update_astc_texture(tex0_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex Update (ASTC)", generation_count);

    return 0.0;
  }

  double vector_field::update_astc_texture_t0(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex0 Update (ASTC)", generation_count);


    update_astc_texture(tex0_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex0 Update (ASTC)", generation_count);

    return 0.0;
  }

  double vector_field::update_astc_texture_t1(gl::GLvoid* data, std::size_t offset_byte, bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Tex1 Update (ASTC)", generation_count);


    update_astc_texture(tex1_ptr, data, offset_byte);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Tex1 Update (ASTC)", generation_count);

    return 0.0;
  }

  double vector_field::update_seeding(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Seeding Update", generation_count);


    b_seeding->setSubData(0, sizeof(glm::vec3), (gl::GLvoid*) & s_conf->stride);
    b_seeding->setSubData(sizeof(glm::vec4), sizeof(glm::vec2), (gl::GLvoid*) & s_conf->range_x);
    b_seeding->setSubData(sizeof(glm::vec4) + sizeof(glm::uvec2), sizeof(glm::vec2), (gl::GLvoid*) & s_conf->range_y);
    b_seeding->setSubData(sizeof(glm::vec4) + 2 * sizeof(glm::uvec2), sizeof(glm::vec2), (gl::GLvoid*) & s_conf->range_z);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Seeding Update", generation_count);

    return 0.0;
  }

  double vector_field::update_integration(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Integration Update", generation_count);

    std::size_t offset = 0;
    b_integration->setSubData(offset, sizeof(glm::vec4), (gl::GLvoid*) & i_conf->grid);
    offset += sizeof(glm::uvec4);
    b_integration->setSubData(offset, sizeof(glm::vec4), (gl::GLvoid*) & i_conf->cell_size);
    offset += sizeof(glm::vec4);
    b_integration->setSubData(offset, 4, (gl::GLvoid*) & i_conf->step_size_h);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*)&i_conf->step_size_dt);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*)&i_conf->dataset_factor);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*) & i_conf->global_step_count);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*) & i_conf->local_step_count);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*)&i_conf->remainder_step_count);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*) & i_conf->strategy);
    offset += 4;
    b_integration->setSubData(offset, 4, (gl::GLvoid*) & i_conf->texelfetch);

    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Integration Update", generation_count);

    return 0.0;
  }

  void vector_field::update_global_time(const std::uint32_t t, const bool last)
  {
    compute_program->setUniform("global_time", t);
    if (last)
      compute_program->setUniform("fin", 1);
    else
      compute_program->setUniform("fin", 0);
  }
  
  void vector_field::update_advection_count()
  {
    generation_count++;
  }

  double vector_field::advect(bool measure_time)
  {
    if (measure_time)
      //p.start_measure_GPU_time(0);
      p.issue_GPU_timestamp("Advection", generation_count);

    gl::glDispatchCompute(s_conf->seeds.x, s_conf->seeds.y, s_conf->seeds.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    if (measure_time)
      //return p.finish_measure_GPU_time(0) / 1000000.0;
      p.issue_GPU_timestamp("Advection", generation_count);

    return 0.0;
  }

  double vector_field::unsteady_advect(std::vector<astc_datatype>& data, bool measure_time)
  {
    const auto& grid    = i_conf->grid;
    const auto& t_until = i_conf->grid.w;
    auto t_slice_size   = t_conf->compressed_byte_size;
    unsigned int compute_pass = 0;

    auto full_passes = (i_conf->local_step_count > 0) ? std::floor((i_conf->global_step_count - i_conf->remainder_step_count) / i_conf->local_step_count + 0.001) : 0;
    bool half_pass = i_conf->remainder_step_count;   

    // The first n-1 advections
    for (unsigned int t = 0; t < full_passes; t++)
    {
      auto offset_t0 = (compute_pass + 0) * t_slice_size;
      auto offset_t1 = (compute_pass + 1) * t_slice_size;
      update_global_time(compute_pass);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (ASTC Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      // Current & next timeslice
      update_astc_texture_t0(data.data(), offset_t0, false);
      update_astc_texture_t1(data.data(), offset_t1, false);
      tex0_ptr->bindActive(gl::GLenum::GL_TEXTURE0);
      tex1_ptr->bindActive(gl::GLenum::GL_TEXTURE1);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (ASTC Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      // Trace for #local_step_count
      advect(false);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      compute_pass++;
    }

    if (half_pass)
    {
      std::cout << "Doing Half Pass" << std::endl;

      auto offset = compute_pass * t_slice_size;
      update_global_time(compute_pass, true);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (ASTC Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      // Last timeslice
      update_astc_texture_t0(data.data(), offset, false);
      tex0_ptr->bindActive(gl::GLenum::GL_TEXTURE0);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (ASTC Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      // Trace for #local_step_count
      advect(false);

      // Grab Timer
      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      compute_pass++;
    }

    return 0.0;
  }


  double vector_field::unsteady_advect(std::vector<float>& data, bool measure_time)
  {
    const auto& grid = i_conf->grid;
    const auto& t_until = i_conf->grid.w;
    auto t_slice_size = grid.x * grid.y * grid.z * 3 * sizeof(float);
    unsigned int compute_pass = 0;

    auto full_passes = (i_conf->local_step_count > 0) ? std::floor((i_conf->global_step_count - i_conf->remainder_step_count) / i_conf->local_step_count + 0.001) : 0;
    bool half_pass = i_conf->remainder_step_count;

    // The first n-1 advections
    for (unsigned int t = 0; t < full_passes; t++)
    {
      auto offset_t0 = (compute_pass + 0) * t_slice_size;
      auto offset_t1 = (compute_pass + 1) * t_slice_size;
      update_global_time(compute_pass);

      if (measure_time)
        //p.start_measure_GPU_time(0);
        p.issue_GPU_timestamp("Unsteady Avection (Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      // Current & next timeslice
      update_texture_t0(data.data(), offset_t0, false);
      update_texture_t1(data.data(), offset_t1, false);
      tex0_ptr->bindActive(gl::GLenum::GL_TEXTURE0);
      tex1_ptr->bindActive(gl::GLenum::GL_TEXTURE1);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      // Trace for #local_step_count
      advect(false);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      compute_pass++;
    }

    if (half_pass)
    {
      auto offset = compute_pass * t_slice_size;
      update_global_time(compute_pass, true);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      // Last timeslice
      update_texture_t0(data.data(), offset, false);
      tex0_ptr->bindActive(gl::GLenum::GL_TEXTURE0);


      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Texture Update " + std::to_string(compute_pass) + ")", generation_count);

      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      // Trace for #local_step_count
      advect(false);

      // Grab Timer
      if (measure_time)
        p.issue_GPU_timestamp("Unsteady Avection (Compute " + std::to_string(compute_pass) + ")", generation_count);

      compute_pass++;
    }
    
    return 0.0;
  }



  advected_field* vector_field::get_result()
  {
    return output.get();
  }

  void vector_field::add_timer(std::string name, double timing, antMenu* menu)
  {
    menu->addUtilTimer(name, generation_count, timing);
  }

  void vector_field::add_unsteady_timer(unsteady_timings timings, antMenu* menu)
  {
    std::string suffix = (c_conf->astc_compressed) ? " [ASTC]" : "";
    menu->addUtilTimer("Advection (" + std::to_string(timings.iterations) + " calls)", generation_count, timings.advections);
    menu->addUtilTimer("Advection (1 call)", generation_count, timings.advection_single);
    menu->addUtilTimer("Texture Uploads (" + std::to_string(timings.iterations * 2) + " calls)", generation_count, timings.texture_uploads);
    menu->addUtilTimer("Texture Uploads (" + std::to_string(2) + " calls)", generation_count, timings.texture_upload_two);
  }

  gl::GLenum vector_field::get_internal_format(int vec_len)
  {
    switch (vec_len)
    {
    case 1:
      return gl::GLenum::GL_R32F;

    case 2:
      return gl::GLenum::GL_RG32F;

    case 3:
      return gl::GLenum::GL_RGB32F;

    case 4:
      return gl::GLenum::GL_RGBA32F;
    }

    // Standard return
    return gl::GLenum::GL_RGB32F;
  }

  gl::GLenum vector_field::get_format(int vec_len)
  {
    switch (vec_len)
    {
    case 1:
      return gl::GLenum::GL_RED;

    case 2:
      return gl::GLenum::GL_RG;

    case 3:
      return gl::GLenum::GL_RGB;

    case 4:
      return gl::GLenum::GL_RGBA;
    }

    // Standard return
    return gl::GLenum::GL_RGB;
  }

  gl::GLenum vector_field::get_internal_astc_format(int blocksize_x, int blocksize_y, int blocksize_z)
  {
    switch (blocksize_x)
    {
    case 4:
      if (blocksize_y == 4)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
      break;

    case 5:
      if (blocksize_y == 4)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
      if (blocksize_y == 5)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
      break;

    case 6:
      if (blocksize_y == 5)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
      if (blocksize_y == 6)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
      break;

    case 8:
      if (blocksize_y == 5)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
      if (blocksize_y == 6)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
      if (blocksize_y == 8)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
      break;

    case 10:
      if (blocksize_y == 5)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
      if (blocksize_y == 6)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
      if (blocksize_y == 8)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
      if (blocksize_y == 10)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
      break;

    case 12:
      if (blocksize_y == 10)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
      if (blocksize_y == 12)
        return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
      break;
    }

    // Standard return
    return gl::GLenum::GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
  }

  gl::GLenum vector_field::get_internal_astc_format(glm::uvec3 block_sizes)
  {
    return get_internal_astc_format(block_sizes.x, block_sizes.y, block_sizes.z);
  }
}