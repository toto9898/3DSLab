#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;
    
    // Material self illumination chunk (0xA084)
    class MatSelfIllumChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SELF_ILLUM"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
    // Material wire size chunk (0xA087)
    class MatWireSizeChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_WIRE_SIZE"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
    // Material shading type chunk (0xA100)
    class MatShadingChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHADING"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
} // namespace Debugger3DS