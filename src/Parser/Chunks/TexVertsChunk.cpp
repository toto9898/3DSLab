#include "TexVertsChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool TexVertsChunk::ReadData(Importer& importer) {
        uint16_t count;
        if (!Read(count)) {
            return false;
        }
        
        // Get reference to current mesh being built
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Read texture coordinates directly into target mesh
        targetMesh_->texCoords.reserve(count);
        for (uint16_t i = 0; i < count; ++i) {
            float u, v;
            if (!Read(u) || !Read(v)) {
                return false;
            }
            targetMesh_->texCoords.emplace_back(u, 1.0f - v);
        }
        
        logging::log << "Texture Vertices: " << count << " texture coordinates" << std::endl;
        return true;
    }

} // namespace Debugger3DS