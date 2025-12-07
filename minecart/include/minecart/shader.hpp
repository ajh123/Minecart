#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <span>
#include <stdexcept>
#include <cstdint>
#include <filesystem>

namespace minecart::graphics {

    // Exception class for shader-related errors
    class ShaderException : public std::runtime_error {
    public:
        explicit ShaderException(const std::string& message)
            : std::runtime_error("Shader error: " + message) {}
    };

    // Custom deleters for SDL GPU shader resources
    struct SDLGPUShaderDeleter {
        SDL_GPUDevice* device = nullptr;
        void operator()(SDL_GPUShader* shader) const noexcept {
            if (shader && device) {
                SDL_ReleaseGPUShader(device, shader);
            }
        }
    };

    struct SDLGPUPipelineDeleter {
        SDL_GPUDevice* device = nullptr;
        void operator()(SDL_GPUGraphicsPipeline* pipeline) const noexcept {
            if (pipeline && device) {
                SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
            }
        }
    };

    // Type aliases for managed resources
    using GPUShaderPtr = std::unique_ptr<SDL_GPUShader, SDLGPUShaderDeleter>;
    using GPUPipelinePtr = std::unique_ptr<SDL_GPUGraphicsPipeline, SDLGPUPipelineDeleter>;

    // Vertex attribute description for pipeline creation
    struct VertexAttribute {
        uint32_t location;
        SDL_GPUVertexElementFormat format;
        uint32_t offset;
    };

    // Configuration for pipeline creation
    struct PipelineConfig {
        uint32_t vertexStride = 0;
        std::span<const VertexAttribute> attributes;
        SDL_GPUPrimitiveType primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        SDL_GPUFillMode fillMode = SDL_GPU_FILLMODE_FILL;
        SDL_GPUCullMode cullMode = SDL_GPU_CULLMODE_NONE;
        SDL_GPUFrontFace frontFace = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
        bool enableBlend = false;
    };

    class Shader {
    public:
        // Constructor - takes non-owning pointers to device and window
        Shader(SDL_GPUDevice* device, SDL_Window* window);
        ~Shader() = default;

        // Prevent copying
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Allow moving
        Shader(Shader&&) noexcept = default;
        Shader& operator=(Shader&&) noexcept = default;

        // Load shaders from HLSL source files (compiled at runtime via SDL_shadercross)
        void load_vertex_shader(const std::filesystem::path& path, const char* entrypoint = "main");
        void load_fragment_shader(const std::filesystem::path& path, const char* entrypoint = "main");

        // Build the graphics pipeline with the given configuration
        void build_pipeline(const PipelineConfig& config);

        // Bind the pipeline for rendering (call before setting uniforms and drawing)
        void bind(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass);

        // Set uniform data for vertex shader at specified slot (0-3)
        template<typename T>
        void set_vertex_uniform(uint32_t slot, const T& data) {
            set_vertex_uniform_raw(slot, &data, sizeof(T));
        }

        // Set uniform data for fragment shader at specified slot (0-3)
        template<typename T>
        void set_fragment_uniform(uint32_t slot, const T& data) {
            set_fragment_uniform_raw(slot, &data, sizeof(T));
        }

        // Check if shader is ready to use
        [[nodiscard]] bool is_ready() const noexcept;

        // Get the pipeline (for advanced use cases)
        [[nodiscard]] SDL_GPUGraphicsPipeline* get_pipeline() const noexcept { return m_pipeline.get(); }

    private:
        void set_vertex_uniform_raw(uint32_t slot, const void* data, uint32_t size);
        void set_fragment_uniform_raw(uint32_t slot, const void* data, uint32_t size);

        SDL_GPUDevice* m_device;    // Non-owning
        SDL_Window* m_window;       // Non-owning

        GPUShaderPtr m_vertexShader;
        GPUShaderPtr m_fragmentShader;
        GPUPipelinePtr m_pipeline;

        // Current command buffer for pushing uniforms
        SDL_GPUCommandBuffer* m_currentCommandBuffer = nullptr;
    };

} // namespace minecart::graphics