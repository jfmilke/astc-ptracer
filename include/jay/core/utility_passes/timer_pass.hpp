#ifndef JAY_GRAPHICS_RENDER_PASSES_TIMER_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_TIMER_PASS_HPP
#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>
#include <chrono>

namespace jay
{
  typedef struct antMenu antMenu;

  struct timer_pass : render_pass
  {
    timer_pass(antMenu* menu);
  };

  JAY_EXPORT timer_pass make_timer_pass(antMenu* menu);
}

#endif