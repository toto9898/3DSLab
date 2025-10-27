#pragma once

#include "Chunk.h"
#include <string>
#include <memory>

namespace Debugger3DS {
    
    // Forward declaration
    struct Material;
    
    // Material name chunk
    class MatNameChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        const std::string& GetMaterialName() const;
        
    private:
        std::string name_;  // Fixed member name
        std::shared_ptr<Material> targetMaterial_ = nullptr;  // Reference to material being built
    };
    
} // namespace Debugger3DS