#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "DelegateHandlers/GeneralSystemEvents.h"

enum CameraMode : uint8_t
{
    None = 0,
    Orthographic = 1,
    Perspective = 2,
    InversePerspective = 3,
};

class Camera
{
public:
    Camera();
    Camera(CameraMode _mode, float _aspect_ratio, float _fov, float _near, float _far, glm::vec3 _eye, glm::vec3 _centre);

    void Update();
    void UpdateOnWindowResize(WindowResizePayload& payload);
public:
    CameraMode mode = CameraMode::Perspective;
    
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 eye;
    glm::vec3 centre;

    glm::mat4 view;
    glm::mat4 projection;

    float fov = glm::radians(45.0f);
    float near_clip_plane = 0.001f;
    float far_clip_plane = 1000.0f;

    float aspect_ratio = 1;
    float speed;
    float yaw = 90.0f;
    float pitch = 0;
};

#endif //!CAMERA_H