#ifndef JAY_GRAPHICS_RENDER_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASS_HPP

#include <functional>

#include <jay/export.hpp>

namespace jay
{
struct JAY_EXPORT render_pass
{
  std::function<void()> on_prepare = [ ] ( ) { };
  std::function<void()> on_update  = [ ] ( ) { };
};
}

#endif