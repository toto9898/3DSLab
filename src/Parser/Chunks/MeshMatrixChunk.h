#pragma once

#include "Chunk.h"
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declaration
    class Mesh;
    
    // Mesh matrix chunk
    class MeshMatrixChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Mesh> targetMesh_ = nullptr;  // Reference to mesh being built
    };
    
} // namespace Debugger3DS