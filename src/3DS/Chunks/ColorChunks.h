#pragma once

#include "Chunk.h"
#include <memory>
#include <type_traits>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declaration
    struct Material;
    
    // Unified color chunk template — reads RGB in the component type T
    // and sets the current material's diffuse color.
    //   T = uint8_t  →  24-bit color (0–255, normalized to 0–1)
    //   T = float    →  float color (already 0–1)
    template<typename T>
    class ColorChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
    };
    
    // Preserve the original type names used by ChunkRegistration
    using Color24Chunk = ColorChunk<uint8_t>;
    using ColorFChunk  = ColorChunk<float>;

} // namespace Debugger3DS
