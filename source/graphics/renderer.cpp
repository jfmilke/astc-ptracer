#include <jay/graphics/renderer.hpp>

namespace jay
{
renderer::renderer()
{
  render_passes_.reserve(20);

  on_prepare = [&] ()
  {
    for (auto& pass : render_passes_)
      pass->on_prepare();
  };
  on_update  = [&] ()
  {
    for (auto& pass : render_passes_)
      pass->on_update ();
  };
}
}
