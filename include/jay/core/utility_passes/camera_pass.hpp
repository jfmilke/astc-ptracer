#ifndef JAY_GRAPHICS_RENDER_PASSES_CAMERA_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_CAMERA_PASS_HPP
#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>

typedef struct GLFWwindow GLFWwindow;

namespace jay
{
  typedef class camera camera;
  typedef class application application;

  struct camera_pass : render_pass
  {
    camera_pass(GLFWwindow* window, camera* cam, application* app);

    GLFWwindow* window_;
    camera* cam_;
    application* app_;
  };

  JAY_EXPORT camera_pass make_camera_pass(GLFWwindow* window, camera* cam, application* app);
}

#endif