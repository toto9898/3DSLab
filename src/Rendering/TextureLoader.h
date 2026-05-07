#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace Debugger3DS {

class TextureLoader {
public:
    // Load a texture from basePath/filename. Returns a cached handle if already loaded.
    bgfx::TextureHandle LoadTexture(const std::string& basePath, const std::string& filename);

    // Set a callback to receive error messages instead of printing to stderr.
    void SetErrorCallback(std::function<void(const std::string&)> callback);

    // Destroy all cached texture handles.
    void Shutdown();

private:
    std::unordered_map<std::string, bgfx::TextureHandle> cache_;
    std::function<void(const std::string&)> onError_;
};

} // namespace Debugger3DS
