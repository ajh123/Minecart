#include "minecart/camera.hpp"

#include <algorithm>

namespace minecart::graphics {

    Camera::Camera() {
        update_vectors();
        update();
    }

    void Camera::set_position(const minecart::maths::Vec3& pos) {
        m_position = pos;
    }

    void Camera::set_position(float x, float y, float z) {
        m_position = minecart::maths::Vec3{x, y, z};
    }

    void Camera::set_rotation(float pitch, float yaw) {
        // Clamp pitch to avoid gimbal lock
        m_pitch = std::clamp(pitch, -1.5533f, 1.5533f);  // ~89 degrees
        m_yaw = yaw;
        update_vectors();
    }

    void Camera::look_at(const minecart::maths::Vec3& target) {
        minecart::maths::Vec3 direction = (target - m_position).normalized();

        // Calculate pitch (vertical angle)
        m_pitch = std::asin(direction.y);

        // Calculate yaw (horizontal angle)
        m_yaw = std::atan2(direction.x, -direction.z);

        update_vectors();
    }

    void Camera::set_perspective(float fovY, float aspect, float nearZ, float farZ) {
        m_fovY = fovY;
        m_aspect = aspect;
        m_nearZ = nearZ;
        m_farZ = farZ;
        m_projectionMatrix = minecart::maths::Mat4::perspective(fovY, aspect, nearZ, farZ);
    }

    void Camera::set_aspect_ratio(float aspect) {
        m_aspect = aspect;
        m_projectionMatrix = minecart::maths::Mat4::perspective(m_fovY, m_aspect, m_nearZ, m_farZ);
    }

    void Camera::move_forward(float distance) {
        m_position = m_position + m_forward * distance;
    }

    void Camera::move_right(float distance) {
        m_position = m_position + m_right * distance;
    }

    void Camera::move_up(float distance) {
        m_position = m_position + minecart::maths::Vec3{0.0f, 1.0f, 0.0f} * distance;
    }

    void Camera::rotate(float deltaPitch, float deltaYaw) {
        m_pitch = std::clamp(m_pitch + deltaPitch, -1.5533f, 1.5533f);
        m_yaw += deltaYaw;
        update_vectors();
    }

    void Camera::update_vectors() {
        // Calculate forward vector from pitch and yaw
        m_forward.x = std::cos(m_pitch) * std::sin(m_yaw);
        m_forward.y = std::sin(m_pitch);
        m_forward.z = -std::cos(m_pitch) * std::cos(m_yaw);
        m_forward = m_forward.normalized();

        // Recalculate right and up vectors
        m_right = minecart::maths::Vec3::cross(m_forward, WORLD_UP).normalized();
        m_up = minecart::maths::Vec3::cross(m_right, m_forward).normalized();
    }

    void Camera::update() {
        // Update view matrix
        minecart::maths::Vec3 target = m_position + m_forward;
        m_viewMatrix = minecart::maths::Mat4::lookAt(m_position.data(), target.data(), WORLD_UP.data());

        // Update projection matrix
        m_projectionMatrix = minecart::maths::Mat4::perspective(m_fovY, m_aspect, m_nearZ, m_farZ);
    }

} // namespace minecart::graphics
