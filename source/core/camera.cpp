#include <jay/core/camera.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <GLFW/glfw3.h>

namespace jay
{
  camera::camera() :
      current(0)
    , lastUpdate(0)
    , deltaTime(0)
    , mouseSpeed(0.1)
    , lastMousePosition(0, 0)
    , deltaMousePosition(0,0)
    , captureMouse(false)
    , cameraSpeed(200.0)
    , deltaCameraPosition(0,0,0)
  {
    cameraStates.reserve(20);
    cameraStates.push_back(cameraState());
  }

  camera::camera(std::vector<std::size_t>& grid, float fov) :
      current(0)
    , lastUpdate(0)
    , deltaTime(0)
    , mouseSpeed(0.1)
    , lastMousePosition(0, 0)
    , deltaMousePosition(0, 0)
    , captureMouse(false)
    , cameraSpeed(200.0)
    , deltaCameraPosition(0, 0, 0)
  {
    cameraStates.reserve(20);

    auto grid_dim = 0;
    auto size = grid.size();

    for (int i = 0; i < size; i++)
      grid_dim = (grid[i] > 1) ? i + 1 : grid_dim;

    // Just for convenience
    glm::vec4 ref_grid;
    ref_grid.x = (grid_dim >= 1) ? grid[0] : 0;
    ref_grid.y = (grid_dim >= 2) ? grid[1] : 0;
    ref_grid.z = (grid_dim >= 3) ? grid[2] : 0;
    ref_grid.w = (grid_dim >= 4) ? grid[3] : 0;

    // Reference points
    float x_mid = ref_grid.x / 2.f;
    float y_mid = ref_grid.y / 2.f;
    float z_mid = ref_grid.z / 2.f;

    // Calculate the correct distance, such that the whole dataset can be seen
    // from 3 different viewpoints

    float ctan = 1.0 / std::tan(fov / 2.0);

    // Frontview
    float z_distance = std::fmax(y_mid * ctan, x_mid * ctan) * 2; // Give it some extra distance
    glm::vec3 c0_pos = glm::vec3(x_mid, y_mid, z_distance);
    glm::vec3 c0_fac = glm::vec3(x_mid, y_mid, 0);
    glm::vec3 c0_up  = glm::vec3(0.0, 1.0, 0.0);

    // 4/3 ratio is a dummy, should be corrected right after camera creation
    cameraStates.push_back(cameraState(c0_pos, c0_fac, c0_up, fov, 4.0 / 3.0, 0.1, 1000.0));
  }


  int camera::addCameraState()
  {
    auto cstate = cameraState();
    cameraStates.push_back(cstate);
    return cameraStates.size() - 1;
  }

  int camera::addCameraState(glm::vec3 cP, glm::vec3 cF, glm::vec3 cU, float foV, float dR, float nC, float fC)
  {
    auto cstate = cameraState(cP, cF, cU, foV, dR, nC, fC);
    cameraStates.push_back(cstate);
    return cameraStates.size() - 1;
  }

  void camera::useCameraState(int id)
  {
    if ((id == 0) && (current > 0))
      current--;
    
    if ((id == 1) && (current < cameraStates.size() - 1 ))
      current++;

    if (id == 99)
      cameraStates[current].reset();
  }

  void camera::lock_direction()
  {
    direction_lock = true;
  }

  void camera::unlock_direction()
  {
    direction_lock = false;
  }

  bool camera::direction_locked()
  {
    return direction_lock;
  }


  void camera::resetCameraState()
  {
    auto& cs = cameraStates[current];

    cs.reset();
  }

  void camera::updateTime(float time)
  {
    deltaTime  = time - lastUpdate;
    lastUpdate = time;
  }

  void camera::updateCamera()
  {
    auto& cs = cameraStates[current];

    if (!direction_lock)
    {
      // Orientation (Mouse)
      cs.cameraYaw += deltaMousePosition.x;
      cs.cameraPitch += deltaMousePosition.y;
      cs.calcDirection();
      // Dampen
      deltaMousePosition *= 0.6;

      // Movement (Keyboard)
      cs.cameraPosition += deltaCameraPosition;
      // Dampen
      deltaCameraPosition *= 0.6;
    }

    // Matrices
    cs.calcMatrices();
  }

  void camera::getCameraConfig(glm::vec3& pos, glm::vec3& dir)
  {
    const auto& cs = cameraStates[current];

    pos = cs.cameraPosition;
    dir = cs.cameraDirection;
  }

  void camera::setCameraConfig(glm::vec3 pos, glm::vec3 dir)
  {
    auto& cs = cameraStates[current];

    cs.cameraPosition = pos;
    cs.cameraDirection = dir;

    lock_direction();
  }


  void camera::setFacePosition(glm::vec3 position, bool all)
  {
    auto& cs = cameraStates[current];

    if (all)
      for (auto& state : cameraStates)
      {
        state.cameraDirection = glm::normalize(position - state.cameraPosition);
        state.cameraPitch     = 90;
        state.cameraYaw       = 0;
        state.cameraRight     = glm::normalize(glm::cross(state.cameraDirection, state.cameraUp));
        state.cameraUp        = glm::normalize(glm::cross(state.cameraDirection, state.cameraRight));

      }

    cs.cameraDirection = glm::normalize(position - cs.cameraPosition);
    cs.cameraPitch = -90;
    cs.cameraYaw = 0;

    // Gram-Schmidt for initialization
    cs.cameraRight = glm::normalize(glm::cross(cs.cameraDirection, cs.cameraUp));
    cs.cameraUp    = glm::normalize(glm::cross(cs.cameraDirection, cs.cameraRight));
  }

  void camera::setPosition(glm::vec3 position, bool all)
  {
    auto& cs = cameraStates[current];

    if (all)
      for (auto& state : cameraStates)
      {
        state.cameraPosition  = position;
        state.cameraFacing   += position;
      }

    cs.cameraPosition  = position;
    cs.cameraFacing   += position;
  }

  void camera::setFoV(float foV, bool all)
  {
    auto& cs = cameraStates[current];

    if (all)
      for (auto& state : cameraStates)
        state.fieldOfView = foV;

    cs.fieldOfView = foV;
  }

  void camera::setDisplayRatio(float displayRatio, bool all)
  {
    auto& cs = cameraStates[current];

    if (all)
      for (auto& state : cameraStates)
        state.changeDisplayRatio(displayRatio);

    cs.changeDisplayRatio(displayRatio);
  }

  void camera::setClipping(float nearClipping, float farClipping, bool all)
  {
    auto& cs = cameraStates[current];

    if (all)
      for (auto& state : cameraStates)
      {
        state.nearClip = nearClipping;
        state.farClip  = farClipping;
      }

    cs.nearClip = nearClipping;
    cs.farClip = farClipping;
  }

  void camera::setCameraSpeed(float speed)
  {
    cameraSpeed = speed;
  }

  void camera::setMouseSpeed(float speed)
  {
    mouseSpeed = speed;
  }

  void camera::startOrientationChange(float mouseX, float mouseY)
  {
    lastMousePosition.x = mouseX;
    lastMousePosition.y = mouseY;

    captureMouse = true;
  }

  void camera::stopOrientationChange()
  {
    captureMouse = false;
  }

  void camera::changeOrientation(float mouseX, float mouseY)
  {
    auto& cs = cameraStates[current];

    deltaMousePosition.x += mouseSpeed * (mouseX - lastMousePosition.x);
    deltaMousePosition.y -= mouseSpeed * (mouseY - lastMousePosition.y);
    lastMousePosition.x = mouseX;
    lastMousePosition.y = mouseY;

    // Restricted Pitch
    if (cs.cameraPitch + deltaMousePosition.y > 89.0f)
      deltaMousePosition.y = 88.0f - cs.cameraPitch;
    
    if (cs.cameraPitch + deltaMousePosition.y < -89.0f)
      deltaMousePosition.y = -88.0f - cs.cameraPitch;
  }

  void camera::changePosition(bool up, bool down, bool forward, bool backward, bool left, bool right)
  {
    auto& cs = cameraStates[current];

    if (up && !down)
      deltaCameraPosition += deltaTime * cameraSpeed * cs.cameraUp;
    if (down && !up)
      deltaCameraPosition -= deltaTime * cameraSpeed * cs.cameraUp;
    if (forward && !backward)
      deltaCameraPosition += deltaTime * cameraSpeed * cs.cameraDirection;
    if (backward && !forward)
      deltaCameraPosition -= deltaTime * cameraSpeed * cs.cameraDirection;
    if (left && !right)
      deltaCameraPosition -= deltaTime * cameraSpeed * cs.cameraRight;
    if (right && !left)
      deltaCameraPosition += deltaTime * cameraSpeed * cs.cameraRight;
  }

  void camera::changeFoV(float deltaFoV)
  {
    cameraStates[current].fieldOfView += deltaFoV;
  }

  glm::mat4 camera::getViewMatrix()
  {
    return cameraStates[current].viewMat;
  }

  glm::mat4 camera::getProjectionMatrix()
  {
    return cameraStates[current].projectionMat;
  }


}
