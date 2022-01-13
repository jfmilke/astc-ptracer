#ifndef JAY_GRAPHICS_RENDER_PASSES_ADVECT_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_ADVECT_PASS_HPP

#include <jay/graphics/render_pass.hpp>
#include <jay/types/jaydata.hpp>
#include <jay/types/image.hpp>
#include <jay/export.hpp>

namespace jay
{  
  typedef struct vector_field vector_field;
  typedef struct antMenu antMenu;

  struct advection_pass : render_pass
  {
    advection_pass(std::vector<float>& data, vector_field* field, antMenu* menu, bool prefer_2darray = false);
    advection_pass(std::vector<astc_datatype>& data, std::vector<float>& denormalization_data, vector_field* field, antMenu* menu);

    // Local variables
    int advection_count;
  };

  JAY_EXPORT advection_pass make_advection_pass(std::vector<float>& data, vector_field* field, antMenu* menu, bool prefer_2darray = false);
  JAY_EXPORT advection_pass make_advection_pass(std::vector<astc_datatype>& data, std::vector<float>& denormalization_data, vector_field* field, antMenu* menu);

}

#endif