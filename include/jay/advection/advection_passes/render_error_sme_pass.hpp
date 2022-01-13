#ifndef JAY_GRAPHICS_RENDER_PASSES_RENDER_ERROR_SME_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_RENDER_ERROR_SME_PASS_HPP

#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>


namespace jay
{  
  typedef struct advected_field advected_field;
  typedef struct antMenu antMenu;
  typedef struct camera camera;


  struct render_error_sme_pass : render_pass
  {
    render_error_sme_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1);
  };

  JAY_EXPORT render_error_sme_pass make_render_error_sme_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1);
}

#endif