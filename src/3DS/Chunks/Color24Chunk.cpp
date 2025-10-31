#include "Color24Chunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool Color24Chunk::ReadData(Importer& importer) {
        // Get current material
        targetMaterial_ = importer.GetCurrentMaterial();
        if (!targetMaterial_) {
            return false;
        }
        
        // Read RGB directly into material diffuse color
        uint8_t r, g, b;
        if (!Read(r) || !Read(g) || !Read(b)) {
            return false;
        }
        
        // Convert to 0-1 range and apply directly
        targetMaterial_->diffuse = Eigen::Vector3f(r / 255.0f, g / 255.0f, b / 255.0f);
        
        logging::log << "Color (24-bit): R=" << (int)r << " G=" << (int)g << " B=" << (int)b << std::endl;
        return true;
    }

} // namespace Debugger3DS