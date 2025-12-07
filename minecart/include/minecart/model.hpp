#pragma once

#include <SDL3/SDL.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <cstdint>
#include <span>

namespace minecart::graphics {

    // Forward declarations
    class Shader;

    // Vertex structure for 3D models
    struct Vertex {
        float position[3];  // x, y, z
        float color[4];     // r, g, b, a

        Vertex() = default;
        Vertex(float x, float y, float z, float r, float g, float b, float a = 1.0f)
            : position{x, y, z}, color{r, g, b, a} {}
    };

    // Exception class for model-related errors
    class ModelException : public std::runtime_error {
    public:
        explicit ModelException(const std::string& message)
            : std::runtime_error("Model error: " + message) {}
    };

    // Custom deleter for SDL GPU buffer
    struct SDLGPUBufferDeleter {
        SDL_GPUDevice* device = nullptr;
        void operator()(SDL_GPUBuffer* buffer) const noexcept {
            if (buffer && device) {
                SDL_ReleaseGPUBuffer(device, buffer);
            }
        }
    };

    // Type alias for managed buffer
    using GPUBufferPtr = std::unique_ptr<SDL_GPUBuffer, SDLGPUBufferDeleter>;

    class Model {
    public:
        // Constructor - takes non-owning pointer to device
        explicit Model(SDL_GPUDevice* device);
        ~Model() = default;

        // Prevent copying
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        // Allow moving
        Model(Model&&) noexcept = default;
        Model& operator=(Model&&) noexcept = default;

        // Set mesh data (vertices and optional indices)
        void set_vertices(std::span<const Vertex> vertices);
        void set_indices(std::span<const uint32_t> indices);

        // Upload data to GPU (call after setting vertices/indices)
        void upload();

        // Render the model (shader must already be bound with uniforms set)
        void render(SDL_GPURenderPass* renderPass) const;

        // Check if model is ready to render
        [[nodiscard]] bool is_ready() const noexcept;

        // Accessors
        [[nodiscard]] uint32_t get_vertex_count() const noexcept { return m_vertexCount; }
        [[nodiscard]] uint32_t get_index_count() const noexcept { return m_indexCount; }
        [[nodiscard]] bool uses_index_buffer() const noexcept { return m_useIndexBuffer; }

    private:
        void upload_vertex_data();
        void upload_index_data();

        SDL_GPUDevice* m_device;        // Non-owning

        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        GPUBufferPtr m_vertexBuffer;
        GPUBufferPtr m_indexBuffer;

        bool m_useIndexBuffer = false;
        bool m_uploaded = false;
        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;
    };

} // namespace minecart::graphics