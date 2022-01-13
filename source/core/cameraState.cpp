#include <jay/core/camera_state.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jay
{
  // Default camera state
  cameraState::cameraState()
    : r_cameraPosition(0, 0, 3)
    , r_cameraFacing(0, 0, 0)
    , r_cameraUp(0, 1, 0)
    , r_fieldOfView(45.0)
    , r_displayRatio(4.0 / 3.0)
    , r_nearClip(0.1)
    , r_farClip(10000.0)
  {
    // Gram-Schmidt for initialization
    //glm::vec3 direction = glm::normalize(r_cameraPosition - r_cameraFacing);
    //glm::vec3 right = glm::normalize(glm::cross(direction, r_cameraUp));
    //r_cameraUp = glm::normalize(glm::cross(direction, right));

    reset();
  }

  // Custom camera state
  cameraState::cameraState(glm::vec3 cP, glm::vec3 cF, glm::vec3 cU, float foV, float dR, float nC, float fC)
    : r_cameraPosition(cP)
    , r_cameraFacing(cF)
    , r_cameraUp(cU)
    , r_fieldOfView(foV)
    , r_displayRatio(dR)
    , r_nearClip(nC)
    , r_farClip(fC)
  {
    // Gram-Schmidt for initialization
    //glm::vec3 direction = glm::normalize(r_cameraPosition - r_cameraFacing);
    //glm::vec3 right     = glm::normalize(glm::cross(direction, r_cameraUp));
    //r_cameraUp = glm::normalize(glm::cross(direction, right));

    reset();
  }

  void cameraState::reset()
  {
    cameraPitch = 0;
    cameraYaw = -90;
    cameraPosition = r_cameraPosition;
    cameraFacing = r_cameraFacing;
    cameraDirection = glm::normalize(cameraPosition - cameraFacing);
    cameraUp = r_cameraUp;
    cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));
    viewMat = glm::lookAt(cameraPosition, cameraDirection, cameraUp);

    fieldOfView = r_fieldOfView;
    displayRatio = r_displayRatio;
    nearClip = r_nearClip;
    farClip = r_farClip;
    projectionMat = glm::perspective(fieldOfView, displayRatio, nearClip, farClip);
  }

  void cameraState::calcDirection()
  {
    glm::vec3 nDirection;
    nDirection.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    nDirection.y = sin(glm::radians(cameraPitch));
    nDirection.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

    cameraDirection = glm::normalize(nDirection);

    // Repair camera coordinate system (Gram-Schmidt)
    //cs.cameraRight = glm::normalize(glm::cross(cs.cameraDirection, cs.cameraUp));
    //cs.cameraUp    = glm::normalize(glm::cross(cs.cameraDirection, cs.cameraRight));
    cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));
  }

  void cameraState::calcMatrices()
  {
    projectionMat = glm::perspective(glm::radians(fieldOfView), displayRatio, nearClip, farClip);
    viewMat       = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
  }

  void cameraState::changeDisplayRatio(float ratio)
  {
    displayRatio = ratio;
    r_displayRatio = ratio;
  }

  void cameraState::save()
  {
    r_cameraPosition = cameraPosition;
    r_cameraFacing = cameraFacing;
    r_cameraUp = cameraUp;

    r_fieldOfView = fieldOfView;
    r_displayRatio = displayRatio;
    r_nearClip = nearClip;
    r_farClip = farClip;
  }
}