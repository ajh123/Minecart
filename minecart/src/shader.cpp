#include "minecart/shader.hpp"

#include <SDL3_shadercross/SDL_shadercross.h>
#include <spdlog/spdlog.h>

#include <vector>
#include <fstream>
#include <sstream>

namespace minecart::graphics {

    // Helper to read file contents
    static std::string read_file_contents(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw ShaderException("Failed to open shader file: " + path.string());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    Shader::Shader(SDL_GPUDevice* device, SDL_Window* window)
        : m_device(device)
        , m_window(window)
        , m_vertexShader(nullptr, SDLGPUShaderDeleter{device})
        , m_fragmentShader(nullptr, SDLGPUShaderDeleter{device})
    {
        if (!device) {
            throw ShaderException("Device cannot be null");
        }
        if (!window) {
            throw ShaderException("Window cannot be null");
        }

        // Initialize SDL_shadercross
        if (!SDL_ShaderCross_Init()) {
            throw ShaderException("Failed to initialize SDL_shadercross");
        }
    }

    void Shader::load_vertex_shader(const std::filesystem::path& path, const char* entrypoint) {
        std::string source = read_file_contents(path);

        // Compile HLSL to SPIR-V
        SDL_ShaderCross_HLSL_Info hlslInfo{};
        hlslInfo.source = source.c_str();
        hlslInfo.entrypoint = entrypoint;
        hlslInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlslInfo.include_dir = path.parent_path().string().c_str();
        hlslInfo.defines = nullptr;
        hlslInfo.props = 0;

        size_t spirvSize = 0;
        void* spirvCode = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &spirvSize);
        if (!spirvCode) {
            throw ShaderException(std::string("Failed to compile vertex shader: ") + SDL_GetError());
        }

        // Reflect shader to get resource info
        SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(
            static_cast<const Uint8*>(spirvCode), spirvSize, 0);
        if (!metadata) {
            SDL_free(spirvCode);
            throw ShaderException(std::string("Failed to reflect vertex shader: ") + SDL_GetError());
        }

        // Create GPU shader from SPIR-V
        SDL_ShaderCross_SPIRV_Info spirvInfo{};
        spirvInfo.bytecode = static_cast<const Uint8*>(spirvCode);
        spirvInfo.bytecode_size = spirvSize;
        spirvInfo.entrypoint = entrypoint;
        spirvInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        spirvInfo.props = 0;

        SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            m_device, &spirvInfo, &metadata->resource_info, 0);

        SDL_free(metadata);
        SDL_free(spirvCode);

        if (!shader) {
            throw ShaderException(std::string("Failed to create vertex shader: ") + SDL_GetError());
        }
        m_vertexShader.reset(shader);
    }

    void Shader::load_fragment_shader(const std::filesystem::path& path, const char* entrypoint) {
        std::string source = read_file_contents(path);

        // Compile HLSL to SPIR-V
        SDL_ShaderCross_HLSL_Info hlslInfo{};
        hlslInfo.source = source.c_str();
        hlslInfo.entrypoint = entrypoint;
        hlslInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        hlslInfo.include_dir = path.parent_path().string().c_str();
        hlslInfo.defines = nullptr;
        hlslInfo.props = 0;

        size_t spirvSize = 0;
        void* spirvCode = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &spirvSize);
        if (!spirvCode) {
            throw ShaderException(std::string("Failed to compile fragment shader: ") + SDL_GetError());
        }

        // Reflect shader to get resource info
        SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(
            static_cast<const Uint8*>(spirvCode), spirvSize, 0);
        if (!metadata) {
            SDL_free(spirvCode);
            throw ShaderException(std::string("Failed to reflect fragment shader: ") + SDL_GetError());
        }

        // Log resource info for debugging
        spdlog::info("Fragment shader '{}' resources: samplers={}, storage_textures={}, storage_buffers={}, uniform_buffers={}",
            path.string(),
            metadata->resource_info.num_samplers,
            metadata->resource_info.num_storage_textures,
            metadata->resource_info.num_storage_buffers,
            metadata->resource_info.num_uniform_buffers);

        // Create GPU shader from SPIR-V
        SDL_ShaderCross_SPIRV_Info spirvInfo{};
        spirvInfo.bytecode = static_cast<const Uint8*>(spirvCode);
        spirvInfo.bytecode_size = spirvSize;
        spirvInfo.entrypoint = entrypoint;
        spirvInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        spirvInfo.props = 0;

        SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            m_device, &spirvInfo, &metadata->resource_info, 0);

        SDL_free(metadata);
        SDL_free(spirvCode);

        if (!shader) {
            throw ShaderException(std::string("Failed to create fragment shader: ") + SDL_GetError());
        }
        m_fragmentShader.reset(shader);
    }

    void Shader::bind(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass, SDL_GPUGraphicsPipeline* pipeline) {
        SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
    }

    void Shader::set_vertex_uniform_raw(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const void* data, uint32_t size) {
        SDL_PushGPUVertexUniformData(commandBuffer, slot, data, size);
    }

    void Shader::set_fragment_uniform_raw(SDL_GPUCommandBuffer* commandBuffer, uint32_t slot, const void* data, uint32_t size) {
        SDL_PushGPUFragmentUniformData(commandBuffer, slot, data, size);
    }

} // namespace minecart::graphics
