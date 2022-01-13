#ifndef JAY_GRAPHICS_RENDER_PASSES_AUTO_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_AUTO_PASS_HPP
#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>
#include <chrono>

namespace jay
{
  typedef struct antMenu antMenu;
  typedef struct camera camera;


  struct auto_pass : render_pass
  {
    auto_pass(antMenu* menu, camera* cam, std::string name);
  };

  JAY_EXPORT auto_pass make_auto_pass(antMenu* menu, camera* cam, std::string name);
}

#endif