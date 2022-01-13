#include <jay/advection/advection_passes/render_advection_pass.hpp>
#include <jay/core/camera.hpp>
#include <jay/core/menu.hpp>
#include <jay/advection/advected_field.hpp>

#include <glbinding/gl/gl.h>
#include <globjects/globjects.h>

#include <iostream>

namespace jay
{

  render_advection_pass::render_advection_pass(antMenu* menu, camera* camera, advected_field* field)
    : advection_count(0)
  {
    on_prepare = [&, menu, camera, field]()
    {
      // Blending
      gl::glEnable(gl::GL_BLEND);
      gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
      //gl::glEnable(gl::GL_FRAMEBUFFER_SRGB);

      // Depth Test
      gl::glEnable(gl::GL_DEPTH_TEST);
      gl::glDepthFunc(gl::GL_LESS);

      gl::glEnable(gl::GL_LINE_SMOOTH);
      gl::glLineWidth(2);

      field->init_configuration(menu);
      field->update_menu(menu);

      field->setup_render_shader();
      field->shader_program->use();

      vsh_t0 = std::filesystem::last_write_time("../shaders/vertex_shader.glsl");
      fsh_t0 = std::filesystem::last_write_time("../shaders/fragment_shader.glsl");
    };
    on_update = [&, menu, camera, field]()
    {
      auto vsh_t1 = std::filesystem::last_write_time("../shaders/vertex_shader.glsl");
      auto fsh_t1 = std::filesystem::last_write_time("../shaders/fragment_shader.glsl");
      if ((vsh_t0 < vsh_t1) || (fsh_t0 < fsh_t1))
      {
        field->update_render_conf();
        field->vertex_shader_source->reload();
        field->fragment_shader_source->reload();

        field->vertex_shader_source->changed();
        field->fragment_shader_source->changed();

        //field->setup_render_shader();
        vsh_t0 = vsh_t1;
        fsh_t0 = fsh_t1;
      }

      field->update_configuration(menu);

      // If a new integration happened:
      if (menu->isDirty())
      {
        // Update the draw range
        field->update_draw_range();

        // Maybe read content of the buffers
        if (false)
        {
          auto content = field->get_positions<glm::vec4>();
          auto relative = content[0];

          for (auto i = 0; i < content.size(); i++)
          {
            const auto c = content[i];

            if (c.x > 2000)
              std::cout << i << std::endl;
          }
        }

        // Don't come here again unless something new happend
        menu->markClean();
      }

      field->shader_program->use();
      field->vao->bind();
      field->enable_array_buffers();

      if (menu->export_positions())
        field->export_positions(menu);

      if (menu->export_velocities())
        field->export_velocities(menu);

      if (menu->change_camera_state())
        menu->set_camera_state(camera);

      menu->update_camera_info(camera);


      // Only draw the first x% of each line
      field->live_update_draw_percent();

      // Have some nice effects on the rendering
      field->live_update_uniforms(camera);

      // Draw the advected particles
      field->draw();
    };
  }

  render_advection_pass make_render_advection_pass(antMenu* menu, camera* camera, advected_field* field)
  {
    return render_advection_pass(menu, camera, field);
  }
}

