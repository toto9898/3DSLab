#pragma once

#include "Chunk.h"
#include <string>
#include <memory>

namespace Debugger3DS {
    
    // Forward declaration
    struct Material;
    class Importer;

    // Base class for material-related chunks that need to access the current material
    class MaterialContainerChunk : public Chunk {
    public:
        using Chunk::Chunk;
        
    protected:
        // Helper to get and validate current material
        bool GetCurrentMaterial(Importer& importer);
        
        std::shared_ptr<Material> targetMaterial_;
    };

    // ============================================================================
    // Material Name Chunk (0xAFFF)
    // ============================================================================
    
    // Material name chunk - creates a new material
    class MatNameChunk : public Chunk {
    public:
        using Chunk::Chunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_NAME"; }
        const std::string& GetMaterialName() const;
        
    private:
        std::string name_;
        std::shared_ptr<Material> targetMaterial_ = nullptr;
    };
    
    // ============================================================================
    // Material Shininess Chunks (0xA040, 0xA041)
    // ============================================================================
    
    // Material shininess chunk (0xA040) - Contains percentage chunks
    class MatShininessChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHININESS"; }
    };
    
    // Material shininess percentage chunk (0xA041) - Shininess as percentage
    class MatShin2PctChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHIN2PCT"; }
    };
    
    // ============================================================================
    // Material Transparency Chunks (0xA050, 0xA052, 0xA053)
    // ============================================================================
    
    // Material transparency chunk (0xA050) - Contains percentage chunks
    class MatTransparencyChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_TRANSPARENCY"; }
    };
    
    // Material transparency falloff chunk (0xA052)
    class MatXpfallChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_XPFALL"; }
    };
    
    // Material reflection blur chunk (0xA053)
    class MatRefblurChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_REFBLUR"; }
    };
    
    // ============================================================================
    // Material Property Chunks (0xA084, 0xA087, 0xA100)
    // ============================================================================
    
    // Material self illumination chunk (0xA084)
    class MatSelfIllumChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SELF_ILLUM"; }
    };
    
    // Material wire size chunk (0xA087)
    class MatWireSizeChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_WIRE_SIZE"; }
    };
    
    // Material shading type chunk (0xA100)
    class MatShadingChunk : public MaterialContainerChunk {
    public:
        using MaterialContainerChunk::MaterialContainerChunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MAT_SHADING"; }
    };
    
} // namespace Debugger3DS
