#ifndef JAY_CORE_APPLICATION_HPP
#define JAY_CORE_APPLICATION_HPP
#define GLFW_CDECL

#include <jay/core/engine.hpp>
#include <jay/core/menu.hpp>
#include <jay/core/camera.hpp>
#include <jay/advection/vector_field.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include <jay/export.hpp>

typedef struct GLFWwindow GLFWwindow;

namespace jay
{
class JAY_EXPORT application : public engine
{
public:
  application                       ();
  application                       (const application&  that) = delete ;
  application                       (      application&& temp) = delete ;
 ~application                       ()                         = default;
  application& operator=            (const application&  that) = delete ;
  application& operator=            (const application&& temp) = delete ;
                                      
  GLFWwindow* window                () const { return window_; }
  
  // Callbacks
  static void key_callback          (GLFWwindow* window, std::int32_t key, std::int32_t scancode, std::int32_t action, std::int32_t mods);
  static void mouse_button_callback (GLFWwindow* window, int button, int action, int mods);
  static void mouse_cursor_callback (GLFWwindow* window, double xpos, double ypos);
  static void mouse_scroll_callback (GLFWwindow* window, double xoffset, double yoffset);
  static void window_size_callback  (GLFWwindow* window, int width, int height);
  static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

  // General
  static float getTime();
  int  getFramebufferHeight();
  int  getFramebufferWidth();
  void run();

  camera*  add_camera();
  camera*  add_camera(std::vector<std::size_t> grid, float fov = 45);
  antMenu* add_menu();
  vector_field* add_vector_field();


  // Input control methods
  bool mouseActive();
  bool keyboardActive();
  void setMouse(bool active);
  void setKeyboard(bool active);

  void switchCameraState(int id);

protected:
  void setOpenGLInfo(antMenu* m);

  GLFWwindow*           window_;
  std::vector<std::unique_ptr<camera>>       cameras_;
  std::vector<std::unique_ptr<antMenu>>      menus_;
  std::vector<std::unique_ptr<vector_field>> fields_;
  bool grabMouse;
  bool grabKeyboard;
  int  framebufferWidth;
  int  framebufferHeight;
};
}

#endif