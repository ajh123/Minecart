#include "minecart/common.hpp"

namespace minecart {

std::string get_minecart_version() {
    #ifdef PACKAGE_VERSION
        return PACKAGE_VERSION;
    #else
        return "unknown";
    #endif
}

} // namespace minecart
