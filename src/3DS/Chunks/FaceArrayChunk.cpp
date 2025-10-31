#include "FaceArrayChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool FaceArrayChunk::ReadData(Importer& importer) {
        
        uint16_t count;
        if (!Read(count)) {
            return false;
        }
        
        // Get reference to current mesh being built
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Read faces directly into target mesh
        targetMesh_->faces.reserve(count);
        for (uint16_t i = 0; i < count; ++i) {
            Face face;
            if (!Read(face.a) ||
                !Read(face.b) ||
                !Read(face.c) ||
                !Read(face.flags)) {
                return false;
            }
            targetMesh_->faces.push_back(face);
        }
        
        logging::log << "Face Array: " << count << " faces" << std::endl;
        return true;
    }

} // namespace Debugger3DS