#ifndef JAY_GRAPHICS_RENDER_PASSES_IMAGE_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_IMAGE_PASS_HPP
#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>

namespace jay
{
  typedef struct antMenu antMenu;

  struct image_pass : render_pass
  {
    image_pass(antMenu* menu);
  };

  JAY_EXPORT image_pass make_image_pass(antMenu* menu);
}

#endif