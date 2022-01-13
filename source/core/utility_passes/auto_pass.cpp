#include <jay/core/utility_passes/auto_pass.hpp>
#include <jay/core/menu.hpp>
#include <jay/core/camera.hpp>
#include <jay/analysis/performance_measure.hpp>
#include <glbinding/gl/gl.h>

namespace jay
{
  auto_pass::auto_pass(antMenu* menu, camera* cam, std::string name)
  {
    on_prepare = [&]()
    {};
    on_update = [&, menu, cam, name]()
    {
      static long int counter = -1;

      if (counter > 11)
        return;

      if (counter == 11)
      {
        menu->export_timers();
        counter++;
      }

      if ((counter > 6) && (counter < 11))
      {
        menu->markDirty();
        counter++;

        return;
      }

      switch (counter)
      {
      case -1:
        break;
      case 0:
        cam->setCameraConfig(glm::vec3(155, 88, 300), glm::vec3(0, 0, -1));
        break;
      case 1:
        cam->setCameraConfig(glm::vec3(155, 80, -180), glm::vec3(0, 0, 1));

        menu->export_pos = true;
        menu->export_velo = true;
        break;
      case 2:
        cam->setCameraConfig(glm::vec3(140, 310, 45), glm::vec3(-0.002, -0.99, 0.06));
        break;
      case 3:
        cam->setCameraConfig(glm::vec3(-110, 92, 30), glm::vec3(1, 0, 0));
        break;
      case 4:
        cam->setCameraConfig(glm::vec3(100, 98, -60), glm::vec3(0.09, -0.13, 0.98));
        break;
      case 5:
        cam->setCameraConfig(glm::vec3(145, 100, -10), glm::vec3(0.7, -0.1, 0.7));
        break;
      case 6:
        cam->setCameraConfig(glm::vec3(70, 90, 140), glm::vec3(0.22, 0, -0.97));
        break;
      }

      if (counter > -1)
      {
        menu->export_img_name = name + "_" + std::to_string(counter) + ".bmp";
        menu->export_img = true;
      }
      

      counter++;
    };
  }

  auto_pass make_auto_pass(antMenu* menu, camera* cam, std::string name)
  {
    return auto_pass(menu, cam, name);
  }
}
