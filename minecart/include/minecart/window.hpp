#pragma once

#include <SDL3/SDL.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlgpu3.h"

#include <memory>
#include <functional>
#include <stdexcept>
#include <string>

namespace minecart::graphics {

    // Custom exception for window-related errors
    class WindowException : public std::runtime_error {
    public:
        explicit WindowException(const std::string& message)
            : std::runtime_error(message) {}
    };

    class SDLException : public WindowException {
    public:
        explicit SDLException(const std::string& context)
            : WindowException(context + ": " + (SDL_GetError() ? SDL_GetError() : "Unknown SDL error")) {}
    };

    class ImGuiException : public WindowException {
    public:
        explicit ImGuiException(const std::string& message)
            : WindowException("ImGui error: " + message) {}
    };

    // Custom deleters for SDL resources (RAII)
    struct SDLWindowDeleter {
        void operator()(SDL_Window* w) const noexcept {
            if (w) SDL_DestroyWindow(w);
        }
    };

    struct SDLGPUDeviceDeleter {
        void operator()(SDL_GPUDevice* d) const noexcept {
            if (d) SDL_DestroyGPUDevice(d);
        }
    };

    // Type aliases for smart pointers
    using SDLWindowPtr = std::unique_ptr<SDL_Window, SDLWindowDeleter>;
    using SDLGPUDevicePtr = std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter>;

    struct FrameContext {
        SDL_GPUCommandBuffer* commandBuffer;
        SDL_GPURenderPass* renderPass;
        SDL_Window* window;
        SDL_GPUDevice* device;
    };

    // Use std::function for more flexibility (lambdas with captures)
    using ImGuiRenderFunction = std::function<void()>;
    using GameRenderFunction = std::function<SDL_AppResult(FrameContext&)>;

    class Window {
    public:
        Window(ImGuiRenderFunction imguiRenderFunc, GameRenderFunction gameRenderFunc);
        ~Window();

        // Prevent copying (Window manages unique resources)
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Allow moving
        Window(Window&&) noexcept = default;
        Window& operator=(Window&&) noexcept = default;

        // Throws WindowException on failure
        SDL_AppResult initialize();
        SDL_AppResult process_event(SDL_Event* event);
        SDL_AppResult render_frame();
        void shutdown() noexcept;

        // Accessors (non-owning pointers)
        [[nodiscard]] SDL_Window* get_window() const noexcept { return window.get(); }
        [[nodiscard]] SDL_GPUDevice* get_device() const noexcept { return device.get(); }
        [[nodiscard]] bool is_initialized() const noexcept { return initialized; }
        [[nodiscard]] SDL_FColor get_clear_color() const noexcept { return clearColor; }

        // Modifiers
        void set_clear_color(const SDL_FColor& color) noexcept { clearColor = color; }

    private:
        SDLWindowPtr window;
        SDLGPUDevicePtr device;
        ImGuiRenderFunction imguiRenderFunction;
        GameRenderFunction gameRenderFunction;
        SDL_FColor clearColor = {0.1f, 0.1f, 0.1f, 1.0f};
        bool initialized = false;
        bool imguiInitialized = false;
    };

} // namespace minecart::graphics