#include "minecart/window.hpp"
#include "minecart/common.hpp"

namespace minecart::graphics {

    Window::Window(ImGuiRenderFunction imguiRenderFunc, GameRenderFunction gameRenderFunc)
        : window(nullptr),
          device(nullptr),
          imguiRenderFunction(std::move(imguiRenderFunc)),
          gameRenderFunction(std::move(gameRenderFunc)),
          initialized(false),
          imguiInitialized(false) {
        
        if (!imguiRenderFunction) {
            throw WindowException("ImGui render function cannot be null");
        }
        if (!gameRenderFunction) {
            throw WindowException("Game render function cannot be null");
        }
    }

    Window::~Window() {
        // Ensure proper cleanup even if shutdown() wasn't called
        if (initialized) {
            shutdown();
        }
    }

    SDL_AppResult Window::initialize() {
        if (initialized) {
            throw WindowException("Window already initialized");
        }

        // Create the window
        SDL_Window* rawWindow = SDL_CreateWindow(
            minecart::get_minecart_version().c_str(), 
            960, 540, 
            SDL_WINDOW_RESIZABLE
        );
        
        if (!rawWindow) {
            throw SDLException("Failed to create SDL window");
        }
        window.reset(rawWindow);

        // Create the GPU device
        SDL_GPUDevice* rawDevice = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV, 
            false, 
            nullptr
        );
        
        if (!rawDevice) {
            window.reset(); // Clean up window before throwing
            throw SDLException("Failed to create GPU device");
        }
        device.reset(rawDevice);

        // Claim window for GPU device
        if (!SDL_ClaimWindowForGPUDevice(device.get(), window.get())) {
            device.reset();
            window.reset();
            throw SDLException("Failed to claim window for GPU device");
        }

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGuiContext* ctx = ImGui::CreateContext();
        if (!ctx) {
            device.reset();
            window.reset();
            throw ImGuiException("Failed to create ImGui context");
        }

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        if (!ImGui_ImplSDL3_InitForSDLGPU(window.get())) {
            ImGui::DestroyContext();
            device.reset();
            window.reset();
            throw ImGuiException("Failed to initialize ImGui SDL3 backend");
        }

        ImGui_ImplSDLGPU3_InitInfo init_info = {};
        init_info.Device = device.get();
        init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device.get(), window.get());
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
        
        if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            device.reset();
            window.reset();
            throw ImGuiException("Failed to initialize ImGui SDL GPU3 backend");
        }

        imguiInitialized = true;
        initialized = true;

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult Window::process_event(SDL_Event* event) {
        if (!initialized) {
            throw WindowException("Window not initialized");
        }

        if (!event) {
            throw WindowException("Event pointer is null");
        }

        // Pass events to ImGui
        ImGui_ImplSDL3_ProcessEvent(event);

        if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            return SDL_APP_SUCCESS;
        }

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult Window::render_frame() {
        if (!initialized) {
            throw WindowException("Window not initialized");
        }

        // Start the ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Call user's ImGui render function
        imguiRenderFunction();

        // Render ImGui
        ImGui::Render();

        // Acquire the command buffer
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device.get());
        if (!commandBuffer) {
            throw SDLException("Failed to acquire GPU command buffer");
        }

        // Get the swapchain texture
        SDL_GPUTexture* swapchainTexture = nullptr;
        Uint32 width = 0, height = 0;
        
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(
                commandBuffer, 
                window.get(), 
                &swapchainTexture, 
                &width, 
                &height)) {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            throw SDLException("Failed to acquire swapchain texture");
        }

        // Handle case where swapchain texture is not available (e.g., minimized window)
        if (!swapchainTexture) {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return SDL_APP_CONTINUE;
        }

        // Prepare ImGui draw data (MUST be called before BeginGPURenderPass)
        ImGui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), commandBuffer);

        // Create the color target
        SDL_GPUColorTargetInfo colorTargetInfo{};
        colorTargetInfo.clear_color = this->get_clear_color();
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapchainTexture;

        // Begin a render pass
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(
            commandBuffer, 
            &colorTargetInfo, 
            1, 
            nullptr
        );
        
        if (!renderPass) {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            throw SDLException("Failed to begin render pass");
        }

        // Draw game content
        FrameContext frameContext {
            commandBuffer,
            renderPass,
            window.get(),
            device.get()
        };

        SDL_AppResult result = gameRenderFunction(frameContext);

        // Render ImGui draw data
        ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderPass);

        // End the render pass
        SDL_EndGPURenderPass(renderPass);

        // Submit the command buffer
        if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
            throw SDLException("Failed to submit GPU command buffer");
        }

        return result;
    }

    void Window::shutdown() noexcept {
        if (!initialized) {
            return;
        }

        // Cleanup ImGui (in reverse order of initialization)
        if (imguiInitialized) {
            ImGui_ImplSDLGPU3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            imguiInitialized = false;
        }

        // Smart pointers will handle SDL resource cleanup automatically
        // but we reset them explicitly for clarity
        device.reset();
        window.reset();

        initialized = false;
    }

} // namespace minecart::graphics