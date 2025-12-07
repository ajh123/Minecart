#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cmath>

#include "minecart/maths.hpp"

namespace minecart::graphics {

    class Camera {
    public:
        Camera();
        ~Camera() = default;

        // Prevent copying
        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;

        // Allow moving
        Camera(Camera&&) noexcept = default;
        Camera& operator=(Camera&&) noexcept = default;

        // Position and orientation
        void set_position(const  minecart::maths::Vec3& pos);
        void set_position(float x, float y, float z);
        [[nodiscard]] const minecart::maths::Vec3& get_position() const noexcept { return m_position; }

        // Euler angles (in radians)
        void set_rotation(float pitch, float yaw);
        [[nodiscard]] float get_pitch() const noexcept { return m_pitch; }
        [[nodiscard]] float get_yaw() const noexcept { return m_yaw; }

        // Look at a target point
        void look_at(const minecart::maths::Vec3& target);

        // Projection settings
        void set_perspective(float fovY, float aspect, float nearZ, float farZ);
        void set_aspect_ratio(float aspect);

        // Movement helpers
        void move_forward(float distance);
        void move_right(float distance);
        void move_up(float distance);

        // Mouse look (delta in pixels, sensitivity is multiplier)
        void rotate(float deltaPitch, float deltaYaw);

        // Update matrices (call after changing position/rotation)
        void update();

        // Get matrices
        [[nodiscard]] const minecart::maths::Mat4& get_view_matrix() const noexcept { return m_viewMatrix; }
        [[nodiscard]] const minecart::maths::Mat4& get_projection_matrix() const noexcept { return m_projectionMatrix; }
        [[nodiscard]] minecart::maths::Mat4 get_view_projection() const { return m_projectionMatrix * m_viewMatrix; }

        // Get the MVP matrix for a given model matrix
        [[nodiscard]] minecart::maths::Mat4 get_mvp(const minecart::maths::Mat4& modelMatrix = minecart::maths::Mat4::identity()) const {
            return m_projectionMatrix * m_viewMatrix * modelMatrix;
        }

        // Direction vectors
        [[nodiscard]] minecart::maths::Vec3 get_forward() const noexcept { return m_forward; }
        [[nodiscard]] minecart::maths::Vec3 get_right() const noexcept { return m_right; }
        [[nodiscard]] minecart::maths::Vec3 get_up() const noexcept { return m_up; }

    private:
        void update_vectors();

        minecart::maths::Vec3 m_position{0.0f, 0.0f, 3.0f};
        float m_pitch{0.0f};  // Up/down rotation
        float m_yaw{-1.5708f};   // Left/right rotation (start looking at -Z)

        minecart::maths::Vec3 m_forward{0.0f, 0.0f, -1.0f};
        minecart::maths::Vec3 m_right{1.0f, 0.0f, 0.0f};
        minecart::maths::Vec3 m_up{0.0f, 1.0f, 0.0f};

        static constexpr minecart::maths::Vec3 WORLD_UP{0.0f, 1.0f, 0.0f};

        // Projection parameters
        float m_fovY{1.0472f};     // 60 degrees in radians
        float m_aspect{16.0f / 9.0f};
        float m_nearZ{0.1f};
        float m_farZ{1000.0f};

        // Cached matrices
        minecart::maths::Mat4 m_viewMatrix;
        minecart::maths::Mat4 m_projectionMatrix;
    };

} // namespace minecart::graphics
