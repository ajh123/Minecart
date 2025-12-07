#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <memory>

#include "minecart/camera.hpp"
#include "minecart/model.hpp"
#include "minecart/shader.hpp"
#include "minecart/window.hpp"


namespace minecart {

/**
 * @brief Get the version string for the Minecart engine
 * @return Version string
 */
std::string get_minecart_version();

/**
 * @brief Abstract base class for game implementations.
 * 
 * Inherit from this class and implement the virtual methods to create
 * a game. The Game class owns the Window and manages the application lifecycle.
 */
class Game {
public:
    Game();
    virtual ~Game();

    // Prevent copying
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Prevent moving (due to unique_ptr and self-reference)
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    /**
     * @brief Main entry point - creates window, runs game loop, and shuts down.
     * @return 0 on success, non-zero on error
     */
    int run();

    /**
     * @brief Get the window instance.
     * @return Reference to the window (only valid after run() starts)
     */
    [[nodiscard]] graphics::Window& get_window();

    /**
     * @brief Called once after the window and GPU device are initialized.
     * 
     * Use this method to load resources, create shaders, models, etc.
     * 
     * @return true on success, false on error
     */
    virtual bool on_init() = 0;

    /**
     * @brief Called every frame to render game content.
     * 
     * This is called during the render pass, after the clear operation.
     * Use the frameContext to access the render pass and submit draw calls.
     * 
     * @param frameContext Contains the command buffer, render pass, window, and device
     * @return true to continue, false to exit
     */
    virtual bool on_render(graphics::FrameContext& frameContext) = 0;

    /**
     * @brief Called for each SDL event.
     * 
     * Use this to handle input events (keyboard, mouse, etc.)
     * 
     * @param event The SDL event to process
     * @return true if the event was handled, false otherwise
     */
    virtual bool on_event(const SDL_Event& event) { return true; }

    /**
     * @brief Called every frame to update game logic.
     * 
     * This is called before rendering each frame.
     * 
     * @param deltaTime Time elapsed since last frame in seconds
     * @return true to continue, false to exit
     */
    virtual bool on_update(float deltaTime) { return true; }

    /**
     * @brief Called every frame to render ImGui content.
     * 
     * This is called during the ImGui frame, after NewFrame() but before Render().
     * Use ImGui:: calls here to create UI elements.
     */
    virtual void on_imgui_render() = 0;

    /**
     * @brief Called during shutdown, before the window is destroyed.
     * 
     * Use this to release game resources (shaders, models, etc.)
     * that depend on the GPU device.
     */
    virtual void on_shutdown() = 0;

    /**
     * @brief Get the version string for the game
     * @return Version string
     */
    virtual std::string get_version() { return "unknown"; }

    /**
     * @brief Get the name of the game
     * @return Game name
     */
    virtual std::string get_name() { return "unknown"; }

private:
    std::unique_ptr<graphics::Window> m_window;
};

} // namespace minecart
