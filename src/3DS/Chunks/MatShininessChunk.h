#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;
    
    // Material shininess chunk (0xA040) - Contains percentage chunks
    class MatShininessChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHININESS"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
    // Material shininess percentage chunk (0xA041) - Shininess as percentage
    class MatShin2PctChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHIN2PCT"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
} // namespace Debugger3DS