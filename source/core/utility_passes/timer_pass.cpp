#include <jay/core/utility_passes/timer_pass.hpp>
#include <jay/core/menu.hpp>
#include <jay/analysis/performance_measure.hpp>
#include <glbinding/gl/gl.h>

namespace jay
{
  timer_pass::timer_pass(antMenu* menu)
  {
    on_prepare = [&]()
    {};
    on_update = [&, menu]()
    {
      // ms per frame
      static int frames = 0;
      static auto lastTime = std::chrono::steady_clock::now();

      auto currentTime = std::chrono::steady_clock::now();
      frames++;

      if ((std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count() >= 1.0))
      {
        auto spf = 1000.0 / frames;
        if ((2 * menu->ms_per_frame) < spf)
          menu->ms_per_frame_lastMax = spf;
        menu->ms_per_frame = spf;
        frames = 0;
        lastTime = currentTime;
      }      

      if (jay::performance::timings_available())
      {
        auto timings = jay::performance::get_GPU_timings();
        for (const auto& t : timings)
        {
          menu->addUtilTimer(t.description, t.pass, t.time_ms, false);
        }

        jay::performance::clear_GPU_timestamps();
      }
    };
  }

  timer_pass make_timer_pass(antMenu* menu)
  {
    return timer_pass(menu);
  }
}
