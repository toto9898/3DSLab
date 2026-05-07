#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "TextureLoader.h"
#include <iostream>

namespace Debugger3DS::Rendering {

bgfx::TextureHandle TextureLoader::LoadTexture(const std::string& basePath, const std::string& filename) {
    // Check cache first
    auto it = cache_.find(filename);
    if (it != cache_.end())
        return it->second;

    std::string fullPath = basePath + "/" + filename;

    int width, height, channels;
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!data) {
        std::cerr << "[Texture] Failed to load '" << fullPath << "': " << stbi_failure_reason() << std::endl;
        cache_[filename] = BGFX_INVALID_HANDLE;
        return BGFX_INVALID_HANDLE;
    }

    const bgfx::Memory* mem = bgfx::copy(data, static_cast<uint32_t>(width * height * 4));
    stbi_image_free(data);

    bgfx::TextureHandle handle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false, // no mipmaps
        1,     // single layer
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC,
        mem
    );

    cache_[filename] = handle;
    return handle;
}

void TextureLoader::Shutdown() {
    for (auto& [name, handle] : cache_) {
        if (bgfx::isValid(handle))
            bgfx::destroy(handle);
    }
    cache_.clear();
}

} // namespace Debugger3DS::Rendering
