#include <jay/core/utility_passes/menu_pass.hpp>
#include <AntTweakBar.h>

namespace jay
{
  menu_pass::menu_pass()
  {
    on_prepare = [&]()
    {};
    on_update = [&]()
    {
      TwDraw();
    };
  }

  menu_pass make_menu_pass()
  {
    return menu_pass();
  }
}
