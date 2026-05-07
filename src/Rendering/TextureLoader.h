#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>

namespace Debugger3DS::Rendering {

/// @brief Cache-backed texture loader for bgfx.
class TextureLoader {
public:
    /// @brief Load a texture from @p basePath / @p filename. Returns a cached handle if already loaded.
    bgfx::TextureHandle LoadTexture(const std::string& basePath, const std::string& filename);

    /// @brief Destroy all cached texture handles.
    void Shutdown();

private:
    std::unordered_map<std::string, bgfx::TextureHandle> cache_;
};

} // namespace Debugger3DS::Rendering::Rendering::Rendering
