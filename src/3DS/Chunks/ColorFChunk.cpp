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
        if (!Read(targetMaterial_->diffuse.x()) ||
            !Read(targetMaterial_->diffuse.y()) ||
            !Read(targetMaterial_->diffuse.z())) {
            return false;
        }
        
        logging::log << "Color (float): R=" << targetMaterial_->diffuse.x() << " G=" << targetMaterial_->diffuse.y() << " B=" << targetMaterial_->diffuse.z() << std::endl;
        return true;
    }

} // namespace Debugger3DS