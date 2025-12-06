#include "minecart/common.hpp"
#include "minecart/window.hpp"

#include <spdlog/spdlog.h>

namespace minecart {

std::string get_minecart_version() {
    #ifdef PACKAGE_VERSION
        return PACKAGE_VERSION;
    #else
        return "unknown";
    #endif
}

Game::Game() = default;

Game::~Game() = default;

int Game::run() {
    try {
        m_window = std::make_unique<graphics::Window>(this);
        SDL_AppResult result = m_window->run();
        m_window.reset();
        return result == SDL_APP_SUCCESS ? 0 : 1;
    }
    catch (const graphics::WindowException& e) {
        spdlog::error("Window Error: {}", e.what());
        return 1;
    }
    catch (const std::exception& e) {
        spdlog::error("Unexpected Error: {}", e.what());
        return 1;
    }
}

graphics::Window& Game::get_window() {
    if (!m_window) {
        throw graphics::WindowException("Window not created yet");
    }
    return *m_window;
}

} // namespace minecart
