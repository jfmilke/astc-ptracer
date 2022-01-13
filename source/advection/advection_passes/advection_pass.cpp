#include <jay/advection/advection_passes/advection_pass.hpp>
#include <jay/core/menu.hpp>
#include <jay/advection/vector_field.hpp>
#include <jay/advection/advected_field.hpp>
#include <iostream>

#include <glbinding/gl/gl.h>

namespace jay
{
  // Uncompressed
advection_pass::advection_pass(std::vector<float>& data, vector_field* field, antMenu* menu, bool prefer_2darray)
  : advection_count(0)
{
  // Steady
  if (field->output->steady_advection)
  {
    on_prepare = [&, field, menu, prefer_2darray]()
    {
      menu->int_unsteady = false;

      field->init_configuration(menu, false, prefer_2darray);
      auto t_cs = field->setup_compute_shader();
      field->compute_program->use();

      auto t_ssbo = field->setup_storage_buffers();
      auto t_ubo  = field->setup_uniform_buffers();
      auto t_tex  = field->setup_textures();

      auto t_tex_up = field->update_texture(data.data(), 0);

      field->update_global_time(0U);

      // Field is not yet advected, but all intial invocations will be summed up under index "-1"
      // The following advection will then be under index "0"
      field->update_advection_count();

      //field->add_timer("Compute Shader", t_cs,     menu);
      //field->add_timer("UBO Setup",      t_ubo,    menu);
      //field->add_timer("SSBO Setup",     t_ssbo,   menu);
      //field->add_timer("Tex Setup",      t_tex,    menu);
      //field->add_timer("Tex Upload",     t_tex_up, menu);
    };
    on_update = [&, field, menu]()
    {
      if (!menu->isDirty())
        return;

      field->update_configuration(menu);
      field->compute_program->use();

      auto t_seed = field->update_seeding();
      auto t_int  = field->update_integration();
      auto t_ssbo = field->update_storage_buffers();

      auto t_advect = field->advect();
      field->update_advection_count();

      //field->add_timer("UBO Upload",  t_seed + t_int, menu);
      //field->add_timer("SSBO Resize", t_ssbo,         menu);
      //field->add_timer("Advection",   t_advect,       menu);
    };
  }
  else
  {
    // Unsteady
    on_prepare = [&, field, menu, prefer_2darray]()
    {
      menu->int_unsteady = true;

      field->init_configuration(menu, false, prefer_2darray);
      auto t_cs = field->setup_compute_shader();
      field->compute_program->use();

      auto t_ssbo = field->setup_storage_buffers();
      auto t_ubo  = field->setup_uniform_buffers();
      auto t_tex  = field->setup_textures();

      field->update_global_time(0);

      // Field is not yet advected, but all intial invocations will be summed up under index "-1"
      // The following advection will then be under index "0"
      field->update_advection_count();

      //field->add_timer("Compute Shader", t_cs,   menu);
      //field->add_timer("UBO Setup",      t_ubo,  menu);
      //field->add_timer("SSBO Setup",     t_ssbo, menu);
      //field->add_timer("Tex Setup",      t_tex,  menu);
    };
    on_update = [&, menu, field]()
    {
      if (!menu->isDirty())
        return;

      field->update_configuration(menu);
      field->compute_program->use();

      auto t_seed = field->update_seeding();
      auto t_int  = field->update_integration();
      auto t_ssbo = field->update_storage_buffers();

      auto t_advect = field->unsteady_advect(data);

      field->update_advection_count();

      //field->add_timer("UBO Upload",  t_seed + t_int, menu);
      //field->add_timer("SSBO Resize", t_ssbo,         menu);
      //field->add_unsteady_timer(t_advect,             menu);
    };
  }
}

// ASTC compressed
advection_pass::advection_pass(std::vector<astc_datatype>& data, std::vector<float>& denormalization_data, vector_field* field, antMenu* menu)
  : advection_count(0)
{
  if (field->output->steady_advection)
  {
    // Steady
    on_prepare = [&, field, menu]()
    {
      menu->int_unsteady = false;

      int componentwise_normalized = denormalization_data.size() / menu->src_grid.z;
      if (menu->src_grid_dim > 3)
        componentwise_normalized /= menu->src_grid.w;

      field->init_configuration(menu, true, true);
      auto t_cs = field->setup_compute_shader(componentwise_normalized == 6);
      field->compute_program->use();

      auto t_ssbo = field->setup_storage_buffers();
      auto t_ubo  = field->setup_uniform_buffers();
      auto t_norm = field->setup_denormalization_buffer();
      auto t_tex  = field->setup_astc_textures();

      auto t_norm_up = field->update_denormalization_buffer(denormalization_data);
      auto t_tex_up  = field->update_astc_texture(data.data());
      field->update_global_time(0);

      // Field is not yet advected, but all intial invocations will be summed up under index "-1"
      // The following advection will then be under index "0"
      field->update_advection_count();

      //field->add_timer("Compute Shader", t_cs,            menu);
      //field->add_timer("UBO Setup",      t_ubo,           menu);
      //field->add_timer("SSBO Setup",     t_ssbo + t_norm, menu);
      //field->add_timer("SSBO Upload",    t_norm_up,       menu);
      //field->add_timer("Tex Setup",      t_tex,           menu);
      //field->add_timer("Tex Upload",     t_tex_up,        menu);
    };
    on_update = [&, field, menu]()
    {
      if (!menu->isDirty())
        return;

      field->update_configuration(menu);
      field->compute_program->use();

      auto t_seed = field->update_seeding();
      auto t_int  = field->update_integration();
      auto t_ssbo = field->update_storage_buffers();

      auto t_advect = field->advect();

      field->update_advection_count();
      

      //field->add_timer("UBO Upload",  t_seed + t_int, menu);
      //field->add_timer("SSBO Resize", t_ssbo,         menu);
      //field->add_timer("Advection",  t_advect,        menu);
    };
  }
  else
  {
    // Unsteady
    on_prepare = [&, field, menu]()
    {
      menu->int_unsteady = true;

      int componentwise_normalized = denormalization_data.size() / menu->src_grid.z;
      if (menu->src_grid_dim > 3)
        componentwise_normalized /= menu->src_grid.w;

      field->init_configuration(menu, true, true);
      auto t_cs = field->setup_compute_shader(componentwise_normalized == 6);
      field->compute_program->use();

      auto t_ssbo = field->setup_storage_buffers();
      auto t_ubo  = field->setup_uniform_buffers();
      auto t_norm = field->setup_denormalization_buffer();
      auto t_tex  = field->setup_astc_textures();

      auto t_norm_up = field->update_denormalization_buffer(denormalization_data);

      field->update_global_time(0);

      field->setup_test_buffer();

      // Field is not yet advected, but all intial invocations will be summed up under index "-1"
      // The following advection will then be under index "0"
      field->update_advection_count();

      //field->add_timer("Compute Shader", t_cs,            menu);
      //field->add_timer("UBO Setup",      t_ubo,           menu);
      //field->add_timer("SSBO Setup",     t_ssbo + t_norm, menu);
      //field->add_timer("SSBO Upload",    t_norm_up,       menu);
      //field->add_timer("Tex Setup",      t_tex,           menu);
    };
    on_update = [&, menu, field]()
    {
      if (!menu->isDirty())
        return;

      field->update_configuration(menu);
      field->compute_program->use();

      auto t_seed = field->update_seeding();
      auto t_int  = field->update_integration();
      auto t_ssbo = field->update_storage_buffers();

      auto t_advect = field->unsteady_advect(data);

      field->update_advection_count();

      //auto content_test = field->output->read_buffer<float>(field->b_test, 0, 0);
      //for (auto i = 0; i < 205; i++)
      //{
      //  std::cout << i << ": " << content_test[i] << std::endl;
      //}

      //field->add_timer("UBO Upload",  t_seed + t_int, menu);
      //field->add_timer("SSBO Resize", t_ssbo,         menu);
      //field->add_unsteady_timer(t_advect,             menu);
    };
  }
}

  advection_pass make_advection_pass(std::vector<float>& data, vector_field* field, antMenu* menu, bool prefer_2darray)
  {
    return advection_pass(data, field, menu, prefer_2darray);
  };

  advection_pass make_advection_pass(std::vector<astc_datatype>& data, std::vector<float>& denormalization_data, vector_field* field, antMenu* menu)
  {
    return advection_pass(data, denormalization_data, field, menu);
  };
}
