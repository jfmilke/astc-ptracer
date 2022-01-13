#include <jay/advection/advected_field.hpp>

#include <jay/io/data_io.hpp>
#include <jay/io/io.hpp>
#include <jay/core/menu.hpp>
#include <jay/core/camera.hpp>
#include <jay/io/image_io.hpp>
#include <glbinding/gl/gl.h>

namespace jay
{
  advected_field::advected_field()
    : vao        (globjects::VertexArray::create())
  { }

  void advected_field::init_configuration(antMenu* menu)
  {
    if (r_conf == nullptr)
      init_render_conf(menu);

    if (s_conf == nullptr)
      init_shading_conf(menu);

    if (d_conf == nullptr)
      init_draw_conf(menu);
  }

  void advected_field::init_render_conf(antMenu* menu)
  {
    r_conf = new render_conf();

    r_conf->vs_code = data_io::read_shader_file("../shaders/vertex_shader.glsl");
    r_conf->fs_code = data_io::read_shader_file("../shaders/fragment_shader.glsl");

    r_conf->pos_index = 0;
    r_conf->vel_index = 1;
  }

  void advected_field::init_draw_conf(antMenu* menu)
  {
    d_conf = new draw_conf();

    d_conf->draw_percent = menu->getDrawPercent();
    d_conf->draw_seeds = menu->draw_seeds;
    d_conf->draw_offset = menu->draw_offset;
    d_conf->base_color = glm::vec4(0.0, 0.0, 1.0, 1.0);
  }

  void advected_field::init_shading_conf(antMenu* menu)
  {
    s_conf = new shading_conf();

    s_conf->min_velo_coloring = menu->shading_min_value;
    s_conf->max_velo_coloring = menu->shading_max_value;
    s_conf->damping = menu->shading_damping_factor;
  }

  void advected_field::update_configuration(antMenu* menu)
  {
    update_shading_conf(menu);
    update_draw_conf(menu);
  }

  void advected_field::update_shading_conf(antMenu* menu)
  {
    s_conf->min_velo_coloring = menu->shading_min_value;
    s_conf->max_velo_coloring = menu->shading_max_value;
    s_conf->damping = menu->shading_damping_factor;
  }

  void advected_field::update_draw_conf(antMenu* menu)
  {
    d_conf->draw_seeds = menu->draw_seeds;
    d_conf->draw_offset = menu->draw_offset;
    d_conf->draw_percent = menu->getDrawPercent();
  }

  void advected_field::update_render_conf()
  {
    r_conf->vs_code = data_io::read_shader_file("../shaders/vertex_shader.glsl");
    r_conf->fs_code = data_io::read_shader_file("../shaders/fragment_shader.glsl");
  }

  void advected_field::setup_render_shader()
  {

    vertex_shader_source = globjects::Shader::sourceFromFile("../shaders/vertex_shader.glsl");
    vertex_shader_source->reload();

    //vertex_shader_source = globjects::Shader::sourceFromString(r_conf->vs_code);
    vertex_shader_template = globjects::Shader::applyGlobalReplacements(vertex_shader_source.get());
    vertex_shader = globjects::Shader::create(gl::GL_VERTEX_SHADER, vertex_shader_template.get());

    if (!r_conf->gs_code.empty())
    {
      geometry_shader_source = globjects::Shader::sourceFromString(r_conf->gs_code);
      geometry_shader_template = globjects::Shader::applyGlobalReplacements(geometry_shader_source.get());
      geometry_shader = globjects::Shader::create(gl::GL_GEOMETRY_SHADER, geometry_shader_template.get());
    }

    fragment_shader_source = globjects::Shader::sourceFromFile("../shaders/fragment_shader.glsl");

    //fragment_shader_source = globjects::Shader::sourceFromString(r_conf->fs_code);
    fragment_shader_template = globjects::Shader::applyGlobalReplacements(fragment_shader_source.get());
    fragment_shader = globjects::Shader::create(gl::GL_FRAGMENT_SHADER, fragment_shader_template.get());

    shader_program.reset();

    shader_program = globjects::Program::create();
    shader_program->attach(vertex_shader.get());
    if (!r_conf->gs_code.empty())
      shader_program->attach(geometry_shader.get());
    shader_program->attach(fragment_shader.get());
    shader_program->link();
  }

  void advected_field::update_menu(antMenu* menu)
  {
    menu->seed_count = r_conf->seed_count;
    menu->draw_seeds = r_conf->seed_count;
    menu->int_global_step_count = r_conf->step_count;
  }

  void advected_field::enable_array_buffers()
  {
    b_positions->bind(gl::GL_ARRAY_BUFFER);
    gl::glVertexAttribPointer(r_conf->pos_index, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, nullptr);
    gl::glEnableVertexAttribArray(r_conf->pos_index);

    b_velocity->bind(gl::GL_ARRAY_BUFFER);
    gl::glVertexAttribPointer(r_conf->vel_index, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, nullptr);
    gl::glEnableVertexAttribArray(r_conf->vel_index);
  }

  void advected_field::setup_triangle_element_array_buffer()
  {
    const auto& seeds = r_conf->seed_count;
    const auto& steps = r_conf->step_count;
    auto vertices = seeds * steps;

    b_indices = globjects::Buffer::create();
    b_indices->bind(gl::GLenum::GL_ELEMENT_ARRAY_BUFFER);

    std::vector<unsigned int> indices((seeds * (steps - 1)) * 6);
    for (unsigned int s = 0; s < seeds; s++) // seeds
      for (unsigned int v = 0; v < steps - 1; v++) // vertices per seed
      {
        unsigned int index = (s * (steps - 1) + v);
        unsigned int vertex = (s * steps + v);
        indices[6U * index + 0U] = vertex;
        indices[6U * index + 1U] = vertex + vertices + 1;
        indices[6U * index + 2U] = vertex + vertices;
        indices[6U * index + 3U] = vertex;
        indices[6U * index + 4U] = vertex + 1;
        indices[6U * index + 5U] = vertex + vertices + 1;
      }

    b_indices->setData(indices, gl::GLenum::GL_STATIC_DRAW);
  }

  void advected_field::setup_triangle_strip_element_array_buffer()
  {
    const auto& seeds = r_conf->seed_count;
    const auto& steps = r_conf->step_count;
    auto vertices = seeds * steps;

    b_indices = globjects::Buffer::create();
    b_indices->bind(gl::GLenum::GL_ELEMENT_ARRAY_BUFFER);

    std::vector<unsigned int> indices(vertices * 2);
    for (auto vertex = 0; vertex < vertices; vertex++)
    {
      indices[2 * vertex + 0] = vertex + vertices;
      indices[2 * vertex + 1] = vertex;
    }
    
    b_indices->setData(indices, gl::GLenum::GL_STATIC_DRAW);

    indexFirst.resize(seeds);
    indexCount.resize(seeds);
    std::uint32_t seed = 0;
    for (auto& first : indexFirst)
    {
      first = (void*)(seed * steps * 2 * sizeof(unsigned int));
      indexCount[seed] = steps * 2;
      seed++;
    }
  }


  void advected_field::enable_element_array_buffer()
  {
    b_indices->bind(gl::GLenum::GL_ELEMENT_ARRAY_BUFFER);
  }

  void advected_field::export_positions(antMenu* menu)
  {
    std::string filepath = "../files/exports/" + menu->export_pos_name_string + ".positions";

    if (data_io::file_exists(filepath))
    {
      menu->export_pos_status = false;
      return;
    }

    auto positions = get_positions<float>();
    std::uint32_t seeds = r_conf->seed_count;


    // Save the seedcount as a header (uint32)
    auto status = data_io::store_binary((char*) &seeds, sizeof(std::uint32_t), (char*)positions.data(), positions.size() * sizeof(float), filepath, true);
    
    if (status == -1)
      menu->export_pos_status = false;
    else
      menu->export_pos_status = true;
  }

  void advected_field::export_image(antMenu* menu)
  {
    gl::GLint dims[4] = { 0 };
    gl::glGetIntegerv(gl::GL_VIEWPORT, dims);
    gl::GLint fbWidth  = dims[2];
    gl::GLint fbHeight = dims[3];

    image* img = new image();
    img->data.resize(fbWidth * fbHeight * 3);
    img->size = { fbWidth, fbHeight };
    gl::glReadPixels(0, 0, fbWidth, fbHeight, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, img->data.data());
    auto status = image_io::export_bmp("../files/exports/img.bmp", img, 3);

    delete img;

    if (status == -1)
      menu->export_img_status = false;
    else
      menu->export_img_status = true;
  }


  void advected_field::export_velocities(antMenu* menu)
  {
    std::string filepath = "../files/exports/" + menu->export_velo_name_string + ".velocities";

    if (data_io::file_exists(filepath))
    {
      menu->export_velo_status = false;
      return;
    }

    auto velocities = get_velocity<float>();
    std::uint32_t seeds = r_conf->seed_count;

    // Save the seedcount as a header (uint32)
    auto status = data_io::store_binary((char*)&seeds, sizeof(std::uint32_t), (char*)velocities.data(), velocities.size() * sizeof(float), filepath, true);
    
    if (status == -1)
      menu->export_velo_status = false;
    else
      menu->export_velo_status = true;
  }

  void advected_field::load_positions(std::string filepath)
  {
    auto filereader = io();
    auto pos   = filereader.read_vector<float>(filepath);
    auto seeds = *reinterpret_cast<std::uint32_t*>(&pos[0]);

    auto vertices = (pos.size() - 1) / 4.0; //each vertex is vec4

    r_conf->seed_count = seeds;
    r_conf->step_count = vertices / seeds;

    b_positions = globjects::Buffer::create();
    b_positions->setData((pos.size() - 1) * sizeof(float), pos.data() + 1, gl::GLenum::GL_STATIC_DRAW);
  }

  void advected_field::load_velocities(std::string filepath)
  {
    auto filereader = io();
    auto velo  = filereader.read_vector<float>(filepath);
    auto seeds = *reinterpret_cast<std::uint32_t*>(&velo[0]);

    auto vertices = (velo.size() - 1) / 4.0; //each vertex is vec4

    r_conf->seed_count = seeds;
    r_conf->step_count = vertices / seeds;

    b_velocity = globjects::Buffer::create();
    b_velocity->setData((velo.size() - 1) * sizeof(float), velo.data() + 1, gl::GLenum::GL_STATIC_DRAW);
  }

  void advected_field::update_draw_range()
  {
    draw_steps = r_conf->step_count;
    draw_seeds = r_conf->seed_count;

    vertexFirst.resize(draw_seeds);
    vertexCount.resize(draw_seeds);

    std::uint32_t id = 0;
    for (auto& val : vertexFirst)
    {
      val = id * draw_steps;
      vertexCount[id] = draw_steps;
      id++;
    }
  }
  
  void advected_field::live_update_uniforms(camera* cam)
  {
    shader_program->use();
    projection = cam->getProjectionMatrix();
    view       = cam->getViewMatrix();

    glm::vec3 cam_pos;
    glm::vec3 cam_dir;
    cam->getCameraConfig(cam_pos, cam_dir);

    shader_program->setUniform("min_velo_threshold", s_conf->min_velo_coloring);
    shader_program->setUniform("max_velo_threshold", s_conf->max_velo_coloring);
    shader_program->setUniform("damping",            s_conf->damping);
    shader_program->setUniform("View",               view);
    shader_program->setUniform("Proj",               projection);
    shader_program->setUniform("base_color",         d_conf->base_color);
    shader_program->setUniform("eye_pos",            cam_pos);
  }

  void advected_field::live_update_draw_percent()
  {
    draw_steps = d_conf->draw_percent * r_conf->step_count;

    if (vertexCount.size() != r_conf->seed_count)
      update_draw_range();

    if (draw_steps != vertexCount[0])
      std::fill(vertexCount.begin(), vertexCount.end(), draw_steps);
  }

  void advected_field::draw()
  {
    const std::uint32_t seeds = r_conf->seed_count;
    const std::uint32_t steps = r_conf->step_count;

    auto o = std::min(d_conf->draw_offset, seeds - 1);
    auto s = std::min(d_conf->draw_seeds, seeds - o);
    
    shader_program->use();
    vao->multiDrawArrays(gl::GL_LINE_STRIP, vertexFirst.data() + o, vertexCount.data() + o, s);

    //shader_program->use();
    //vao->multiDrawArrays(gl::GL_LINE_STRIP, vertexFirst.data(), vertexCount.data(), seeds);
  }

  void advected_field::draw_triangle_elements()
  {
    const std::uint32_t seeds = r_conf->seed_count;
    const std::uint32_t steps = r_conf->step_count;
    const std::uint32_t verts = (steps - 1) * 6;

    auto o = std::min(d_conf->draw_offset * verts, seeds * verts - verts);
    auto s = std::min(d_conf->draw_seeds * verts, seeds * verts - o);

    shader_program->use();
    vao->drawElements(gl::GL_TRIANGLES, s, gl::GL_UNSIGNED_INT, (void*) (o*sizeof(gl::GLuint)));
  }

  void advected_field::draw_triangle_strip_elements()
  {
    const std::uint32_t seeds = r_conf->seed_count;
    const std::uint32_t steps = r_conf->step_count;
    const std::uint32_t verts = (steps - 1) * 6;

    const auto o = std::min(d_conf->draw_offset, seeds - 1);
    const auto s = std::min(d_conf->draw_seeds, seeds - o);

    shader_program->use();
    gl::glMultiDrawElements(gl::GLenum::GL_TRIANGLE_STRIP, indexCount.data(), gl::GLenum::GL_UNSIGNED_INT, indexFirst.data() + o, s);
  }
}