#include "MaterialChunks.h"
#include "Importer.h"
#include "Material.h"
#include "Logger.h"

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;

    //==============================================================================
    // Material Name
    //==============================================================================

    bool MaterialContainerChunk::GetCurrentMaterial(Importer& importer) {
        targetMaterial_ = importer.GetCurrentMaterial();
        return targetMaterial_ != nullptr;
    }

    bool MaterialContainerChunk::ReadData(Importer& importer) {
        if (!GetCurrentMaterial(importer)) {
            return false;
        }
        
        logging::log << GetTypeName() << " container" << std::endl;
        return true;
    }

    bool MatNameChunk::ReadData(Importer& importer) {
        if (!Read(name_)) {
            return false;
        }
        
        // Create new material and set as current
        targetMaterial_ = std::make_shared<Material>(name_);
        importer.GetScene().materials.push_back(targetMaterial_);
        importer.SetCurrentMaterial(targetMaterial_);
        importer.SetCurrentMaterialName(name_);
        
        logging::log << "Material Name: " << name_ << std::endl;
        return true;
    }

    const std::string& MatNameChunk::GetMaterialName() const {
        return name_;
    }

    //==============================================================================
    // Material Shininess
    //==============================================================================

    //==============================================================================
    // Material Transparency
    //==============================================================================

    //==============================================================================
    // Material Properties
    //==============================================================================

    bool MatWireSizeChunk::ReadData(Importer& importer) {
        if (!GetCurrentMaterial(importer)) {
            return false;
        }
        
        float wireSize;
        if (!Read(wireSize)) {
            return false;
        }
        
        targetMaterial_->wireSize = wireSize;
        
        logging::log << "Material Wire Size: " << wireSize << std::endl;
        return true;
    }

    bool MatShadingChunk::ReadData(Importer& importer) {
        if (!GetCurrentMaterial(importer)) {
            return false;
        }
        
        uint16_t shadingType;
        if (!Read(shadingType)) {
            return false;
        }
        
        targetMaterial_->shadingType = shadingType;
        
        const char* shadingName;
        switch (shadingType) {
            case 0: shadingName = "Flat"; break;
            case 1: shadingName = "Gouraud"; break;
            case 2: shadingName = "Phong"; break;
            case 3: shadingName = "Metal"; break;
            default: shadingName = "Unknown"; break;
        }
        
        logging::log << "Material Shading Type: " << shadingType << " (" << shadingName << ")" << std::endl;
        return true;
    }

    //==============================================================================
    // Texture Map
    //==============================================================================

    bool MatMapNameChunk::ReadData(Importer& importer) {
        if (!GetCurrentMaterial(importer)) {
            return false;
        }
        
        std::string mapName;
        if (!Read(mapName)) {
            return false;
        }
        
        targetMaterial_->textureMap = mapName;
        
        logging::log << "Material Texture Map: " << mapName << std::endl;
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks
