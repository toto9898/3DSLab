#include "MatTransparencyChunk.h"
#include "../Importer.h"
#include "../SceneObjects/Material.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MatTransparencyChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // MAT_TRANSPARENCY is a container chunk - children contain the actual values
        logging::log << "Material Transparency container" << std::endl;
        return true;
    }
    
    bool MatXpfallChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // This chunk should be handled by INT_PERCENTAGE or FLOAT_PERCENTAGE child chunks
        // We don't read data directly here
        logging::log << "Material Transparency Falloff container" << std::endl;
        return true;
    }
    
    bool MatRefblurChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // This chunk should be handled by INT_PERCENTAGE or FLOAT_PERCENTAGE child chunks
        // We don't read data directly here
        logging::log << "Material Reflection Blur container" << std::endl;
        return true;
    }

} // namespace Debugger3DS