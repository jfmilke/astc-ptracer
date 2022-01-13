#include <jay/core/application.hpp>

#include <cstdint>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <globjects/globjects.h>
#include <GLFW/glfw3.h>

#include <jay/graphics/renderer.hpp>

#define _windowW 2560
#define _windowH 1440

// Application
// ===========
namespace jay
{
application::application()
{
  // Initialize GLFW.
  glfwInit      ();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE);

  // Create window.
  window_      = glfwCreateWindow(_windowW, _windowH, "Jay", nullptr, nullptr);
  cameras_.reserve(5);
  menus_.reserve(5);
  fields_.reserve(5);

  glfwSetWindowUserPointer(window_, this);

	glfwSetKeyCallback            (window_, key_callback);
  glfwSetMouseButtonCallback    (window_, mouse_button_callback);
  glfwSetCursorPosCallback      (window_, mouse_cursor_callback);
  glfwSetScrollCallback         (window_, mouse_scroll_callback);
  glfwSetCharCallback           (window_, (GLFWcharfun)TwEventCharModsGLFW3cdecl); // direct AntTweakBar CB
  //glfwSetWindowSizeCallback     (window_, window_size_callback);
  //glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

  // Initialize OpenGL.
  glfwMakeContextCurrent(window_);
  glfwSwapInterval      (1);
  glbinding::initialize (glfwGetProcAddress);
  globjects::init       (glfwGetProcAddress);
  globjects::Buffer     ::hintBindlessImplementation (globjects::Buffer ::BindlessImplementation::DirectStateAccessEXT);
  globjects::Texture    ::hintBindlessImplementation (globjects::Texture::BindlessImplementation::DirectStateAccessEXT);
  globjects::VertexArray::hintAttributeImplementation(globjects::VertexArray::AttributeImplementation::DirectStateAccessARB);

  // Initialize AntTweakBar
  TwInit(TW_OPENGL_CORE, NULL);
  TwWindowSize(_windowW, _windowH);

  // Initialize rest
  framebufferHeight = _windowH;
  framebufferWidth  = _windowW;
  grabKeyboard      = false;
  grabMouse         = false;

  // Create renderer.
  //add_system<renderer>();
}

// General
// =======
void application::run()
{
  for (auto& system : systems_)
    system->on_prepare();
  while (!glfwWindowShouldClose(window_))
    for (auto& system : systems_)
      system->on_update();
}

float application::getTime() {
  return glfwGetTime();
}
int application::getFramebufferHeight() {
  return framebufferHeight;
}
int application::getFramebufferWidth() {
  return framebufferWidth;
}
void application::setMouse(bool active) {
  grabMouse = active;
}
void application::setKeyboard(bool active) {
  grabKeyboard = active;
}
bool application::mouseActive() {
  return grabMouse;
}
bool application::keyboardActive() {
  return grabKeyboard;
}

camera* application::add_camera()
{
  cameras_.push_back(std::make_unique<camera>());
  const auto cam = cameras_.back().get();
  cam->setDisplayRatio((float)framebufferWidth / (float)framebufferHeight, true);
  return static_cast<camera*>(cam);
}

camera* application::add_camera(std::vector<std::size_t> grid, float fov)
{
  cameras_.push_back(std::make_unique<camera>(grid, fov));
  const auto cam = cameras_.back().get();
  cam->setDisplayRatio((float)framebufferWidth / (float)framebufferHeight, true);
  return static_cast<camera*>(cam);
}

antMenu* application::add_menu()
{
  menus_.push_back(std::make_unique<antMenu>());
  const auto menu = menus_.back().get();
  setOpenGLInfo(menu);
  return static_cast<antMenu*>(menu);
}

vector_field* application::add_vector_field()
{
  fields_.push_back(std::make_unique<vector_field>());
  const auto field = fields_.back().get();
  return static_cast<vector_field*>(field);
}

void application::switchCameraState(int id)
{
  auto cam = cameras_.back().get();

  if (cam != nullptr)
    cam->useCameraState(id);
}

void application::setOpenGLInfo(antMenu* m)
{
  auto result = new gl::GLint64[1];

  m->ogl_support_4d_tex            = glfwExtensionSupported("GL_SGIS_texture4D") + glfwExtensionSupported("SGIS_texture4D");
  m->ogl_support_astc_ldr          = glfwExtensionSupported("GL_KHR_texture_compression_astc_ldr");
  m->ogl_support_astc_sliced       = glfwExtensionSupported("KHR_texture_compression_astc_sliced_3d");
  m->ogl_support_astc_hdr          = glfwExtensionSupported("GL_KHR_texture_compression_astc_hdr");

  gl::glGetInteger64v(gl::GLenum::GL_MAX_3D_TEXTURE_SIZE, result);
  m->ogl_max_size_3dtex            = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_ARRAY_TEXTURE_LAYERS, result);
  m->ogl_max_layers_2darraytex     = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_RECTANGLE_TEXTURE_SIZE, result);
  m->ogl_max_size_2dtex            = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_TEXTURE_BUFFER_SIZE, result);
  m->ogl_max_size_texturebuffers   = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_TEXTURE_IMAGE_UNITS, result);
  m->ogl_max_count_textures        = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_NUM_COMPRESSED_TEXTURE_FORMATS, result);
  m->ogl_count_compression_formats = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, result);
  m->ogl_cs_max_ssbos              = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_COMPUTE_UNIFORM_BLOCKS, result);
  m->ogl_cs_max_ubos               = result[0];

  gl::glGetInteger64v(gl::GLenum::GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, result);
  m->ogl_cs_max_textures           = result[0];

  gl::glGetInteger64i_v(gl::GLenum::GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, result);
  m->ogl_cs_max_workgroups_x       = result[0];

  gl::glGetInteger64i_v(gl::GLenum::GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, result);
  m->ogl_cs_max_workgroups_y       = result[0];

  gl::glGetInteger64i_v(gl::GLenum::GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, result);
  m->ogl_cs_max_workgroups_z       = result[0];

  m->calcOGLInfoStrs();

  delete[] result;
}

bool checkExtensionSupport(const char* extension_name)
{
  return glfwExtensionSupported(extension_name);
}


// Callbacks
// =========
void application::key_callback(GLFWwindow* window, std::int32_t key, std::int32_t scancode, std::int32_t action, std::int32_t mods)
{
  // General Controls
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    TwTerminate();
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  // AntTweakBar
  TwEventKeyGLFW3cdecl(window, key, scancode, action, mods);

  int cs = -1;
  if ((key == GLFW_KEY_F1 && action == GLFW_PRESS))
    cs = 0;
  if ((key == GLFW_KEY_F2 && action == GLFW_PRESS))
    cs = 1;
  if ((key == GLFW_KEY_F10 && action == GLFW_PRESS))
    cs = 99;

  if (cs > -1)
  {
    application* app = static_cast<application*>(glfwGetWindowUserPointer(window));

    for (const auto& camera : app->cameras_)
    {
      if (camera != nullptr)
        camera->useCameraState(cs);
    }
  }


  // Camera: Positional Controls
  bool movementDetected = false;
  if ((key == GLFW_KEY_SPACE) && action == GLFW_PRESS)
    movementDetected = true;
  if ((key == GLFW_KEY_LEFT_CONTROL) && action == GLFW_PRESS)
    movementDetected = true;
  if ((key == GLFW_KEY_W) && action == GLFW_PRESS)
    movementDetected = true;
  if ((key == GLFW_KEY_S) && action == GLFW_PRESS)
    movementDetected = true;
  if ((key == GLFW_KEY_A) && action == GLFW_PRESS)
    movementDetected = true;
  if ((key == GLFW_KEY_D) && action == GLFW_PRESS)
    movementDetected = true;

  if (movementDetected)
  {
    application* app = static_cast<application*>(glfwGetWindowUserPointer(window));

    app->setKeyboard(movementDetected);
  }
}

void application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  // AntTweakBar
  TwEventMouseButtonGLFW3cdecl(window, button, action, mods);

  // Camera: Orientational Controls
  bool orientationDetected = false;
  if ((button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
    orientationDetected = true;

  if (orientationDetected)
  {
    application* app = static_cast<application*>(glfwGetWindowUserPointer(window));
    app->setMouse(orientationDetected);
  }
}

void application::mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
  TwEventCursorPosGLFW3cdecl(window, xpos, ypos);
}

void application::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  TwEventScrollGLFW3cdecl(window, xoffset, yoffset);
}

void application::window_size_callback(GLFWwindow* window, int width, int height)
{
  TwWindowSize(width, height);
}

void application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  application* app = static_cast<application*>(glfwGetWindowUserPointer(window));

  app->framebufferHeight = height;
  app->framebufferWidth = width;

  for (auto& cam : app->cameras_)
    cam->setDisplayRatio(float(width) / float(height), true);
}
}
