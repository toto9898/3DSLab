#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;
    
    // Material transparency chunk (0xA050) - Contains percentage chunks
    class MatTransparencyChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_TRANSPARENCY"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
    // Material transparency falloff chunk (0xA052)
    class MatXpfallChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_XPFALL"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
    // Material reflection blur chunk (0xA053)
    class MatRefblurChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_REFBLUR"; }
        
    private:
        std::shared_ptr<Material> targetMaterial_;
    };
    
} // namespace Debugger3DS