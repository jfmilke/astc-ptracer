#ifndef JAY_GRAPHICS_CAMERA_HPP
#define JAY_GRAPHICS_CAMERA_HPP
#include <jay/core/camera_state.hpp>
#include <jay/core/system.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include <jay/export.hpp>

/* Camera
 * ======
 * This simple camera setup uses Euler angles for navigation.
 * That can theoretically lead to the infamous Gimbal Lock, which is why we:
 *   - permit camera rolling              (gimbal locked and known)
 *   - allow restricted pitch-maneuvers   (pitch gimbal doesn't align with yaw-gimbal)
 *   - allow unrestricted yaw-maneuvers
 *
 * To simply use the camera call:
 *    main()
 *       ..
 *       application->startCamera();
 *       auto cam = application->getCamera();
 *       ..
 *    
 *    render()
 *       ..
 *       view = cam->getViewMatrix();
 *       proj = cam->getProjectionMatrix();
 *       ..
 * 
 * The camera API allows for:
 *     - Manipulation of the current camera-setup and its parameters
 *     - Manual management of the camera system (instead of startCamera())
 *     - TODO: Saving, resetting and returning to camera configurations
 *         
 */
namespace jay
{
class JAY_EXPORT camera : public system
{
  public:
    camera();
    camera(std::vector<std::size_t>& grid, float fov);
    camera(const camera& that) = delete;
    camera(camera&& temp) = delete;
   ~camera() = default;
    camera& operator=(const camera& that) = delete;
    camera& operator=(camera&& temp) = delete;

    // Updates the camera systems time.
    // Call once a frame before updateCamera.
    void updateTime(float time);

    // Updates the camera and matrices based on the given input.
    // Call once a frame after updateTime.
    void updateCamera();

    // Adds the default camera state.
    int addCameraState();
    // Adds a custom camera state.
    int addCameraState(glm::vec3 cP, glm::vec3 cF, glm::vec3 cU, float foV, float dR, float nC, float fC);

    void useCameraState(int id);
    void resetCameraState();

    // Manipulate the camera parameters and its state
    void setFacePosition(glm::vec3 position, bool all = false);
    void setPosition(glm::vec3 position, bool all = false);
    void setFoV(float foV, bool all = false);
    void setDisplayRatio(float displayRatio, bool all = false);
    void setClipping(float nearClipping, float farClipping, bool all = false);
    void setCameraSpeed(float speed);
    void setMouseSpeed(float speed);

    void getCameraConfig(glm::vec3& pos, glm::vec3& dir);
    void setCameraConfig(glm::vec3 pos, glm::vec3 dir);

    // Orientate only on event (e.g. rightclick)
    // Call once the event happened.
    void startOrientationChange(float mouseX, float mouseY);
    // Orientate only until event (e.g. released rightclick)
    // Call once the event happened.
    void stopOrientationChange();

    // Lock direction will make the camera use the saved direction vector instead of calculating it from pitch and yaw.
    // This enables reproducable camera viewports.
    void lock_direction();
    void unlock_direction();
    bool direction_locked();


    // Change the camera state continously
    void changeOrientation(float mouseX, float mouseY);
    void changePosition(bool up, bool down, bool forward, bool backward, bool left, bool right);
    void changeFoV(float foV);

    // Get the transformation matrices based on the current camera state
    // Call after updateCamera.
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    bool      captureMouse = true;

    std::vector<cameraState> cameraStates;
    //cameraState&             cs;
    int current = 0;
private:
    // Camera state management
    bool direction_lock = false;

    // Time
    float     lastUpdate = 0.0;
    float     deltaTime = 0.0;

    // Input:
    float     mouseSpeed = 10.0;
    glm::vec2 lastMousePosition = glm::vec2(0,0);
    glm::vec2 deltaMousePosition = glm::vec2(0, 0);

    // Input: Keyboard
    float     cameraSpeed = 50.0;
    glm::vec3 deltaCameraPosition = glm::vec3(0, 0, 0);
};
}

#endif