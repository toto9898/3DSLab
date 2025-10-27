#pragma once

#include "Chunk.h"
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declaration
    struct Material;
    
    // Color chunks (24-bit RGB)
    class Color24Chunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor
        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Material> targetMaterial_ = nullptr;  // Reference to material being modified
    };
    
} // namespace Debugger3DS