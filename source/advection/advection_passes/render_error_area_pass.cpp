#include <jay/advection/advection_passes/render_error_area_pass.hpp>
#include <jay/core/camera.hpp>
#include <jay/core/menu.hpp>
#include <jay/advection/advected_field.hpp>
#include <jay/io/data_io.hpp>
#include <jay/analysis/distance_measure.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/gl/types.h>
#include <globjects/globjects.h>

#include <iostream>

namespace jay
{

  render_error_area_pass::render_error_area_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1)
  {
    on_prepare = [&, menu, camera, field0, field1]()
    {
      gl::glEnable(gl::GL_BLEND);
      gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
      gl::glEnable(gl::GL_DEPTH_TEST);
      gl::glDepthFunc(gl::GL_LESS);
      gl::glDisable(gl::GL_CULL_FACE);

      // Don't need the velocity if it was loaded
      if (field0->b_velocity != nullptr)
        field0->b_velocity->detach();
      if (field1->b_velocity != nullptr)
        field1->b_velocity->detach();

      // If necessary initizalize
      field0->init_configuration(menu);
      field1->init_configuration(menu);

      const auto& steps = field0->r_conf->step_count;
      const auto& seeds = field0->r_conf->seed_count;
      auto vertices = seeds * steps; // originally
      auto bytes = vertices * 4 * sizeof(float); // originally
      auto triangles = 2 * seeds * (steps - 1);

      // Use modified shaders
      field0->r_conf->vs_code = data_io::read_shader_file("../shaders/_tests/render_divergion/vertex_shader.glsl");
      field0->r_conf->fs_code = data_io::read_shader_file("../shaders/_tests/render_divergion/fragment_shader.glsl");
      //field0->r_conf->gs_code = data_io::read_shader_file("../shaders/_tests/render_divergion/geometry_shader.glsl");

      // Start setting up rendering pipeline
      field0->setup_render_shader();
      field0->shader_program->use();
      
      // Get the positional data of the buffers to calculate error area
      auto p0 = field0->get_positions<float>();
      auto p1 = field1->get_positions<float>();
      auto s_area = jay::calculate_seed_area(p0, p1, seeds);
      p0.clear();
      p1.clear();
      
      // Merge the content of both position buffers in a single one
      // 1. Move some pointers
      field0->b_utility1 = std::move(field0->b_positions);
      field0->b_utility2 = std::move(field1->b_positions);
      
      // 2. Create a buffer with enough memory for the input
      field0->b_positions = globjects::Buffer::create();
      field0->b_positions->bind(gl::GLenum::GL_COPY_WRITE_BUFFER);
      field0->b_positions->setData(2 * bytes, NULL, gl::GLenum::GL_STATIC_COPY);

      // 3. Copy the content into the new buffer
      field0->b_utility1->bind(gl::GLenum::GL_COPY_READ_BUFFER);
      field0->b_utility1->copySubData(field0->b_positions.get(), 0, 0, bytes);
      field0->b_utility1->unbind(gl::GLenum::GL_COPY_READ_BUFFER);
      field0->b_utility2->bind(gl::GLenum::GL_COPY_READ_BUFFER);
      field0->b_utility2->copySubData(field0->b_positions.get(), 0, bytes, bytes);
      field0->b_utility2->unbind(gl::GLenum::GL_COPY_READ_BUFFER);

      // 4. Delete the old buffers
      field0->b_utility1->detach();
      field0->b_utility2->detach();

      // Upload the error area
      field0->b_utility0 = globjects::Buffer::create();
      field0->b_utility0->bind(gl::GL_SHADER_STORAGE_BUFFER);
      field0->b_utility0->setData(s_area, gl::GLenum::GL_STATIC_DRAW);

      // Calculate & upload indices into the error area buffer
      std::vector<unsigned int> area_indices(2 * vertices);
      for (auto b = 0; b < 2; b++)
      {
        std::size_t offset = b * vertices;
        for (auto i = 0; i < vertices; i++)
        {
          area_indices[offset + i] = i / steps;
        }
      }

      field0->b_utility1 = globjects::Buffer::create();
      field0->b_utility1->bind(gl::GL_SHADER_STORAGE_BUFFER);
      field0->b_utility1->setData(area_indices, gl::GLenum::GL_STATIC_DRAW);

      // Vertex indices
      field0->setup_triangle_element_array_buffer();

      // Update external settings
      field0->update_menu(menu);
    };
    on_update = [&, menu, camera, field0, field1]()
    {
      field0->shader_program->use();

      field0->vao->bind();
      field0->b_positions->bind(gl::GL_ARRAY_BUFFER);
      gl::glVertexAttribPointer(0, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, nullptr);
      gl::glEnableVertexAttribArray(0);
      
      field0->b_utility0->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 0);
      field0->b_utility1->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 1);

      field0->enable_element_array_buffer();

      field0->update_draw_conf(menu);
      if (menu->change_camera_state())
        menu->set_camera_state(camera);

      menu->update_camera_info(camera);

      field0->live_update_uniforms(camera);

      field0->draw_triangle_elements();
    };
  }

  render_error_area_pass make_render_error_area_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1)
  {
    return render_error_area_pass(menu, camera, field0, field1);
  }
}

