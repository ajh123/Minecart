#pragma once

#include <array>
#include <cmath>

namespace minecart::maths {
    // Simple 4x4 matrix struct (column-major for GPU compatibility)
    struct Mat4 {
        float m[16];

        Mat4() : m{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1} {}

        static Mat4 identity() {
            return Mat4{};
        }

        static Mat4 perspective(float fovY, float aspect, float nearZ, float farZ) {
            Mat4 result{};
            float tanHalfFov = std::tan(fovY * 0.5f);

            result.m[0] = 1.0f / (aspect * tanHalfFov);
            result.m[5] = 1.0f / tanHalfFov;
            result.m[10] = -(farZ + nearZ) / (farZ - nearZ);
            result.m[11] = -1.0f;
            result.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
            result.m[15] = 0.0f;

            return result;
        }

        static Mat4 lookAt(const float* eye, const float* center, const float* up) {
            // Calculate forward, right, up vectors
            float f[3] = {
                center[0] - eye[0],
                center[1] - eye[1],
                center[2] - eye[2]
            };
            // Normalize f
            float fLen = std::sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
            f[0] /= fLen; f[1] /= fLen; f[2] /= fLen;

            // Right = f x up
            float r[3] = {
                f[1]*up[2] - f[2]*up[1],
                f[2]*up[0] - f[0]*up[2],
                f[0]*up[1] - f[1]*up[0]
            };
            float rLen = std::sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
            r[0] /= rLen; r[1] /= rLen; r[2] /= rLen;

            // Recalculate up = r x f
            float u[3] = {
                r[1]*f[2] - r[2]*f[1],
                r[2]*f[0] - r[0]*f[2],
                r[0]*f[1] - r[1]*f[0]
            };

            Mat4 result{};
            result.m[0] = r[0];
            result.m[1] = u[0];
            result.m[2] = -f[0];
            result.m[3] = 0.0f;

            result.m[4] = r[1];
            result.m[5] = u[1];
            result.m[6] = -f[1];
            result.m[7] = 0.0f;

            result.m[8] = r[2];
            result.m[9] = u[2];
            result.m[10] = -f[2];
            result.m[11] = 0.0f;

            result.m[12] = -(r[0]*eye[0] + r[1]*eye[1] + r[2]*eye[2]);
            result.m[13] = -(u[0]*eye[0] + u[1]*eye[1] + u[2]*eye[2]);
            result.m[14] = (f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2]);
            result.m[15] = 1.0f;

            return result;
        }

        static Mat4 translation(float x, float y, float z) {
            Mat4 result{};
            result.m[12] = x;
            result.m[13] = y;
            result.m[14] = z;
            return result;
        }

        static Mat4 rotationY(float angle) {
            Mat4 result{};
            float c = std::cos(angle);
            float s = std::sin(angle);
            result.m[0] = c;
            result.m[2] = s;
            result.m[8] = -s;
            result.m[10] = c;
            return result;
        }

        static Mat4 rotationX(float angle) {
            Mat4 result{};
            float c = std::cos(angle);
            float s = std::sin(angle);
            result.m[5] = c;
            result.m[6] = s;
            result.m[9] = -s;
            result.m[10] = c;
            return result;
        }

        // Matrix multiplication
        Mat4 operator*(const Mat4& other) const {
            Mat4 result;
            for (int i = 0; i < 16; i++) result.m[i] = 0;

            for (int col = 0; col < 4; col++) {
                for (int row = 0; row < 4; row++) {
                    for (int k = 0; k < 4; k++) {
                        result.m[col * 4 + row] += m[k * 4 + row] * other.m[col * 4 + k];
                    }
                }
            }
            return result;
        }

        const float* data() const { return m; }
    };

    // Vec3 helper struct
    struct Vec3 {
        float x, y, z;

        constexpr Vec3() : x(0), y(0), z(0) {}
        constexpr Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

        Vec3 operator+(const Vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
        Vec3 operator-(const Vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
        Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }

        float length() const { return std::sqrt(x*x + y*y + z*z); }

        Vec3 normalized() const {
            float len = length();
            if (len > 0.0001f) return {x/len, y/len, z/len};
            return {0, 0, 0};
        }

        static Vec3 cross(const Vec3& a, const Vec3& b) {
            return {
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        }

        const float* data() const { return &x; }
    };
} // namespace minecart::maths