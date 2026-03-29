#pragma once

#include "Chunk.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declaration
    class Mesh;
    
    // Texture vertices chunk
    class TexVertsChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Mesh> targetMesh_ = nullptr;  // Reference to mesh being built
    };
    
} // namespace Debugger3DS