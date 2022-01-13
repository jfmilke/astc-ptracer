#ifndef JAY_GRAPHICS_RENDER_PASSES_RENDER_ADVECTION_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_RENDER_ADVECTION_PASS_HPP

#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>
#include <filesystem>




namespace jay
{  
  typedef struct advected_field advected_field;
  typedef struct antMenu antMenu;
  typedef struct camera camera;


  struct render_advection_pass : render_pass
  {
    render_advection_pass(antMenu* menu, camera* camera, advected_field* field);

    // Local variables
    std::size_t advection_count;
    std::filesystem::file_time_type vsh_t0;
    std::filesystem::file_time_type fsh_t0;

  };

  JAY_EXPORT render_advection_pass make_render_advection_pass(antMenu* menu, camera* camera, advected_field* field);
}

#endif