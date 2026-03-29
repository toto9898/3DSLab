#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;
    
    // Enhanced integer percentage chunk that can handle different material contexts
    class IntPercentageChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "INT_PERCENTAGE"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
        
        // Helper to determine what property to set based on parent chunk
        void SetMaterialProperty(int16_t percentage, uint16_t parentChunkId);
    };
    
    // Enhanced float percentage chunk that can handle different material contexts  
    class FloatPercentageChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "FLOAT_PERCENTAGE"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
        
        // Helper to determine what property to set based on parent chunk
        void SetMaterialProperty(float percentage, uint16_t parentChunkId);
    };
    
} // namespace Debugger3DS