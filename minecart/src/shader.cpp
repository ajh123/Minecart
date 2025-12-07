#include "minecart/shader.hpp"

#include <SDL3_shadercross/SDL_shadercross.h>

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
        , m_pipeline(nullptr, SDLGPUPipelineDeleter{device})
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

    void Shader::build_pipeline(const PipelineConfig& config) {
        if (!m_vertexShader || !m_fragmentShader) {
            throw ShaderException("Both vertex and fragment shaders must be loaded before building pipeline");
        }

        // Build vertex attributes array
        std::vector<SDL_GPUVertexAttribute> vertexAttributes;
        vertexAttributes.reserve(config.attributes.size());

        for (const auto& attr : config.attributes) {
            SDL_GPUVertexAttribute sdlAttr{};
            sdlAttr.location = attr.location;
            sdlAttr.buffer_slot = 0;
            sdlAttr.format = attr.format;
            sdlAttr.offset = attr.offset;
            vertexAttributes.push_back(sdlAttr);
        }

        // Vertex buffer description
        SDL_GPUVertexBufferDescription vertexBufferDesc{};
        vertexBufferDesc.slot = 0;
        vertexBufferDesc.pitch = config.vertexStride;
        vertexBufferDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vertexBufferDesc.instance_step_rate = 0;

        SDL_GPUVertexInputState vertexInputState{};
        vertexInputState.vertex_buffer_descriptions = &vertexBufferDesc;
        vertexInputState.num_vertex_buffers = 1;
        vertexInputState.vertex_attributes = vertexAttributes.data();
        vertexInputState.num_vertex_attributes = static_cast<Uint32>(vertexAttributes.size());

        // Color target description
        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = SDL_GetGPUSwapchainTextureFormat(m_device, m_window);
        colorTargetDesc.blend_state.enable_blend = config.enableBlend;
        colorTargetDesc.blend_state.color_write_mask = 0xF; // RGBA

        // Create pipeline
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = m_vertexShader.get();
        pipelineInfo.fragment_shader = m_fragmentShader.get();
        pipelineInfo.vertex_input_state = vertexInputState;
        pipelineInfo.primitive_type = config.primitiveType;

        // Rasterizer state
        pipelineInfo.rasterizer_state.fill_mode = config.fillMode;
        pipelineInfo.rasterizer_state.cull_mode = config.cullMode;
        pipelineInfo.rasterizer_state.front_face = config.frontFace;

        // Multisample state
        pipelineInfo.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
        pipelineInfo.multisample_state.sample_mask = 0xFFFFFFFF;

        // Color targets
        pipelineInfo.target_info.color_target_descriptions = &colorTargetDesc;
        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.has_depth_stencil_target = false;

        SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(m_device, &pipelineInfo);
        if (!pipeline) {
            throw ShaderException(std::string("Failed to create graphics pipeline: ") + SDL_GetError());
        }
        m_pipeline.reset(pipeline);
    }

    void Shader::bind(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass) {
        if (!m_pipeline) {
            throw ShaderException("Pipeline not built - call build_pipeline() first");
        }
        m_currentCommandBuffer = commandBuffer;
        SDL_BindGPUGraphicsPipeline(renderPass, m_pipeline.get());
    }

    void Shader::set_vertex_uniform_raw(uint32_t slot, const void* data, uint32_t size) {
        if (!m_currentCommandBuffer) {
            throw ShaderException("Shader not bound - call bind() first");
        }
        SDL_PushGPUVertexUniformData(m_currentCommandBuffer, slot, data, size);
    }

    void Shader::set_fragment_uniform_raw(uint32_t slot, const void* data, uint32_t size) {
        if (!m_currentCommandBuffer) {
            throw ShaderException("Shader not bound - call bind() first");
        }
        SDL_PushGPUFragmentUniformData(m_currentCommandBuffer, slot, data, size);
    }

    bool Shader::is_ready() const noexcept {
        return m_pipeline != nullptr;
    }

} // namespace minecart::graphics
