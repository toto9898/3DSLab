#pragma once

#include "Chunk.h"
#include <vector>
#include <memory>
#include "Mesh.h"

namespace Debugger3DS {
    
    // Forward declaration
    class Mesh;

    // Face array chunk
    class FaceArrayChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor
        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Mesh> targetMesh_ = nullptr;  // Reference to mesh being built
    };
    
} // namespace Debugger3DS