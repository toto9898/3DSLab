#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>

namespace Debugger3DS {

class TextureLoader {
public:
    // Load a texture from basePath/filename. Returns a cached handle if already loaded.
    bgfx::TextureHandle LoadTexture(const std::string& basePath, const std::string& filename);

    // Destroy all cached texture handles.
    void Shutdown();

private:
    std::unordered_map<std::string, bgfx::TextureHandle> cache_;
};

} // namespace Debugger3DS
