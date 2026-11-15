#pragma once

#include <filesystem>

namespace psychic_ui {

    /**
     * The running executable's directory (`/proc/self/exe` on Linux, `_NSGetExecutablePath` on macOS,
     * `GetModuleFileNameW` on Windows), or an empty path when it can't be found
     */
    std::filesystem::path executableDirectory();

}
