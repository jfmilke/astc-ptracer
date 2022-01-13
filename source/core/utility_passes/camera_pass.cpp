#include <jay/core/utility_passes/camera_pass.hpp>
#include <jay/core/application.hpp>
#include <GLFW/glfw3.h>


namespace jay
{
  camera_pass::camera_pass(GLFWwindow* window, camera* cam, application* app)
    : window_(window)
    , cam_(cam)
    , app_(app)
  {
    on_prepare = [&]()
    {};
    on_update = [&, window, cam, app]()
    {
      cam->updateTime((float)glfwGetTime());

      if (app->mouseActive())
      {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (!cam->captureMouse)
        {
          cam->unlock_direction();
          cam->startOrientationChange(mouseX, mouseY);
        }

        cam->changeOrientation(mouseX, mouseY);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
        {
          cam->captureMouse = false;
          app->setMouse(false);
        }
      }

      if (app->keyboardActive())
      {
        bool moveUp = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool moveDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
        bool moveForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool moveBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool moveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool moveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

        cam->changePosition(moveUp, moveDown, moveForward, moveBackward, moveLeft, moveRight);

        if (!(moveUp || moveDown || moveForward || moveBackward || moveLeft || moveRight))
          app->setKeyboard(false);
      }

      cam->updateCamera();
    };
  }

  camera_pass make_camera_pass(GLFWwindow* window, camera* cam, application* app)
  {
    return camera_pass(window, cam, app);
  };
}