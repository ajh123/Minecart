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

    // Type aliases for managed resources
    using GPUShaderPtr = std::unique_ptr<SDL_GPUShader, SDLGPUShaderDeleter>;

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

        // Get the shaders (for pipeline creation elsewhere)
        [[nodiscard]] SDL_GPUShader* get_vertex_shader() const noexcept { return m_vertexShader.get(); }
        [[nodiscard]] SDL_GPUShader* get_fragment_shader() const noexcept { return m_fragmentShader.get(); }

        // Bind the pipeline for rendering (call before setting uniforms and drawing)
        void bind(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass, SDL_GPUGraphicsPipeline* pipeline);

        // Set uniform data for vertex shader at specified slot (0-3)
        template<typename T>
        void set_vertex_uniform(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const T& data) {
            set_vertex_uniform_raw(commandBuffer, slot, &data, sizeof(T));
        }

        // Set uniform data for fragment shader at specified slot (0-3)
        template<typename T>
        void set_fragment_uniform(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const T& data) {
            set_fragment_uniform_raw(commandBuffer, slot, &data, sizeof(T));
        }

    private:
        void set_vertex_uniform_raw(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const void* data, uint32_t size);
        void set_fragment_uniform_raw(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const void* data, uint32_t size);

        SDL_GPUDevice* m_device;    // Non-owning
        SDL_Window* m_window;       // Non-owning

        GPUShaderPtr m_vertexShader;
        GPUShaderPtr m_fragmentShader;
    };

} // namespace minecart::graphics