#ifndef JAY_GRAPHICS_CAMERA_STATE_HPP
#define JAY_GRAPHICS_CAMERA_STATE_HPP
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace jay
{
  struct cameraState
  {
    // Input: Mouse specifics
    float cameraPitch;
    float cameraYaw;

    // View Matrix
    glm::vec3 cameraPosition;
    glm::vec3 cameraFacing;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    glm::mat4 viewMat;

    // Projection Matrix
    float     fieldOfView;
    float     displayRatio;
    float     nearClip;
    float     farClip;
    glm::mat4 projectionMat;

    // default
    cameraState();
    // custom
    cameraState(glm::vec3 cPos, glm::vec3 cFace, glm::vec3 cUp, float foV, float dRatio, float nClip, float fClip);

    // Computes camera direction
    void calcDirection();

    // Computes the view and projection matrix
    void calcMatrices();

    void changeDisplayRatio(float ratio);

    // Saves the current state as reset values
    void save();

    // Resets the camera state
    void reset();


    // Reset values
  private:
    // View Matrix
    glm::vec3 r_cameraPosition;
    glm::vec3 r_cameraFacing;
    glm::vec3 r_cameraUp;

    // Projection Matrix
    float     r_fieldOfView;
    float     r_displayRatio;
    float     r_nearClip;
    float     r_farClip;
  };
}

#endif