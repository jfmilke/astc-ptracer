#ifndef JAY_GRAPHICS_RENDER_PASSES_MENU_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_MENU_PASS_HPP
#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>

namespace jay
{
  struct menu_pass : render_pass
  {
    menu_pass();
  };

  JAY_EXPORT menu_pass make_menu_pass();
}

#endif