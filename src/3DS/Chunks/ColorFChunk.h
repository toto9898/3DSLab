#pragma once

#include "Chunk.h"
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declaration
    struct Material;
    
    // Color chunks (float RGB)
    class ColorFChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor
        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Material> targetMaterial_ = nullptr;  // Reference to material being modified
    };
    
} // namespace Debugger3DS