#include "ColorFChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool ColorFChunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read RGB directly into material diffuse color
        if (!ReadFloat(targetMaterial_->diffuse.x()) ||
            !ReadFloat(targetMaterial_->diffuse.y()) ||
            !ReadFloat(targetMaterial_->diffuse.z())) {
            return false;
        }
        
        logging::log << "Color (float): R=" << targetMaterial_->diffuse.x() << " G=" << targetMaterial_->diffuse.y() << " B=" << targetMaterial_->diffuse.z() << std::endl;
        return true;
    }

} // namespace Debugger3DS