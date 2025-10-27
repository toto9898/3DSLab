#pragma once

#include "Chunk.h"
#include <vector>
#include <string>
#include <memory>

namespace Debugger3DS {
    
    // Forward declarations
    class Mesh;
    struct Material;
    
    // Mesh material group chunk
    class MshMatGroupChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        
    private:
        std::shared_ptr<Mesh> targetMesh_ = nullptr;       // Reference to mesh being modified
        std::shared_ptr<Material> targetMaterial_ = nullptr; // Reference to material being assigned
    };
    
} // namespace Debugger3DS