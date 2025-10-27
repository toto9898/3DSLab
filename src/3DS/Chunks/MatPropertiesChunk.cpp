#include "MatPropertiesChunk.h"
#include "../Importer.h"
#include "../SceneObjects/Material.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MatSelfIllumChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // This chunk should be handled by INT_PERCENTAGE or FLOAT_PERCENTAGE child chunks
        // We don't read data directly here
        logging::log << "Material Self Illumination container" << std::endl;
        return true;
    }
    
    bool MatWireSizeChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read wire size as float
        float wireSize;
        if (!ReadFloat(wireSize)) {
            return false;
        }
        
        targetMaterial_->wireSize = wireSize;
        
        logging::log << "Material Wire Size: " << wireSize << std::endl;
        return true;
    }
    
    bool MatShadingChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read shading type as 16-bit integer
        uint16_t shadingType;
        if (!ReadUShort(shadingType)) {
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

} // namespace Debugger3DS