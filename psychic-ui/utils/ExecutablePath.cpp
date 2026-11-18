#include <algorithm>
#include <string>
#include <system_error>
#include "ExecutablePath.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elifdef __APPLE__
#include <cstdint>
#include <mach-o/dyld.h>
#endif

namespace psychic_ui {

    std::filesystem::path executableDirectory() {
        #ifdef _WIN32
        std::wstring buffer(MAX_PATH, L'\0');
        while (true) {
            DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (length == 0) {
                return {};
            }
            if (length < buffer.size()) {
                buffer.resize(length);
                break;
            }
            buffer.resize(buffer.size() * 2); // Truncated: try again with more room
        }
        std::filesystem::path executable{buffer};
        #elifdef __APPLE__
        std::uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // Sets the size it needs
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
            return {};
        }
        buffer.resize(std::min(buffer.find('\0'), buffer.size()));
        // The path the executable was started by, which may go through symbolic links or `..`
        std::error_code       error;
        std::filesystem::path executable = std::filesystem::weakly_canonical(buffer, error);
        if (error) {
            return {};
        }
        #elifdef __linux__
        std::error_code       error;
        const std::filesystem::path executable = std::filesystem::read_symlink("/proc/self/exe", error);
        if (error) {
            return {};
        }
        #else
        std::filesystem::path executable{};
        #endif
        return executable.parent_path();
    }

}
