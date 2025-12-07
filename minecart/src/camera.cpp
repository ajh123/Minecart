#include "minecart/camera.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace minecart::graphics {

    constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    Camera::Camera() {
        update();
    }

    void Camera::set_position(const glm::vec3& pos) {
        m_position = pos;
        update();
    }

    void Camera::set_position(float x, float y, float z) {
        m_position = glm::vec3{x, y, z};
        update();
    }

    void Camera::set_rotation(float pitch, float yaw) {
        // Clamp pitch to avoid gimbal lock
        m_pitch = std::clamp(pitch, -1.5533f, 1.5533f);  // ~89 degrees
        m_yaw = yaw;
        update();
    }

    void Camera::look_at(const glm::vec3& target) {
        glm::vec3 direction = glm::normalize(target - m_position);

        // Calculate pitch (vertical angle)
        m_pitch = std::asin(direction.y);

        // Calculate yaw (horizontal angle)
        m_yaw = std::atan2(direction.x, -direction.z);

        update();
    }

    void Camera::set_perspective(float fovY, float aspect, float nearZ, float farZ) {
        m_fovY = fovY;
        m_aspect = aspect;
        m_nearZ = nearZ;
        m_farZ = farZ;
        m_projectionMatrix = glm::perspective(fovY, aspect, nearZ, farZ);
    }

    void Camera::set_aspect_ratio(float aspect) {
        m_aspect = aspect;
        m_projectionMatrix = glm::perspective(m_fovY, m_aspect, m_nearZ, m_farZ);
    }

    void Camera::move_forward(float distance) {
        m_position += m_forward * distance;
        update();
    }

    void Camera::move_right(float distance) {
        m_position += m_right * distance;
        update();
    }

    void Camera::move_up(float distance) {
        m_position += WORLD_UP * distance;
        update();
    }

    void Camera::rotate(float deltaPitch, float deltaYaw) {
        m_pitch = std::clamp(m_pitch + deltaPitch, -1.5533f, 1.5533f);
        m_yaw += deltaYaw;
        update();
    }

    void Camera::update() {
        // Calculate forward vector from pitch and yaw
        m_forward.x = std::sin(m_yaw) * std::cos(m_pitch);
        m_forward.y = std::sin(m_pitch);
        m_forward.z = -std::cos(m_yaw) * std::cos(m_pitch);
        m_forward = glm::normalize(m_forward);

        // Recalculate right and up vectors
        m_right = glm::normalize(glm::cross(m_forward, WORLD_UP));
        m_up = glm::normalize(glm::cross(m_right, m_forward));

        m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, WORLD_UP);
        // m_projectionMatrix is updated elsewhere
    }

} // namespace minecart::graphics
