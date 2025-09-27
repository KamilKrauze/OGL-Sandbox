#include "Camera.h"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "Logger.hpp"

Camera::Camera()
{
    this->projection = glm::perspective(fov, aspect_ratio, near_clip_plane, far_clip_plane);

    this->eye = glm::vec3(0, 0, 4);
    this->centre = glm::vec3(0, 0, -1);

    view = glm::lookAt(eye, centre, up);

    this->speed = 0.05f;
}

Camera::Camera(CameraMode _mode, float _aspect_ratio, float _fov, float _near, float _far, glm::vec3 _eye, glm::vec3 _centre)
{
    this->eye = _eye;
    this->centre = _centre;

    view = glm::lookAt(_eye, _centre, up);

    this->aspect_ratio = _aspect_ratio;
    this->speed = 0.05f;
    this->fov = _fov;
    this->near_clip_plane = _near;
    this->far_clip_plane = _far;
    
    this->projection = glm::perspective(fov, aspect_ratio, near_clip_plane, far_clip_plane);
}

void Camera::Update()
{
    view = glm::lookAt(eye, eye + centre, up);
    
    switch (mode)
    {
        case CameraMode::Perspective:
            projection = glm::perspective(fov, aspect_ratio, near_clip_plane, far_clip_plane); break;
        case CameraMode::Orthographic:
            projection = glm::ortho(fov, aspect_ratio, near_clip_plane, far_clip_plane); break;
        case CameraMode::InversePerspective:
            projection = glm::inverse(glm::perspective(fov, aspect_ratio, near_clip_plane, far_clip_plane)); break;
        default:
            return;
    }
}

void Camera::UpdateOnWindowResize(WindowResizePayload& payload)
{
    aspect_ratio = (float)payload.width/(float)payload.height;
    this->Update();
}
