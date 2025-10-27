#include "MatShininessChunk.h"
#include "../Importer.h"
#include "../SceneObjects/Material.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MatShininessChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // MAT_SHININESS is a container chunk - children contain the actual values
        logging::log << "Material Shininess container" << std::endl;
        return true;
    }
    
    bool MatShin2PctChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // This chunk should be handled by INT_PERCENTAGE or FLOAT_PERCENTAGE child chunks
        // We don't read data directly here
        logging::log << "Material Shininess Percentage container" << std::endl;
        return true;
    }

} // namespace Debugger3DS