#include "MshMatGroupChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MshMatGroupChunk::ReadData(Importer& importer) {
        // Read material name (but use current material instead of searching)
        std::string materialName;
        if (!Read(materialName)) {
            return false;
        }
        
        // Get references to current mesh and current material
        targetMesh_ = importer.GetCurrentMesh();
        targetMaterial_ = importer.GetCurrentMaterial();
        
        if (!targetMesh_ || !targetMaterial_) {
            return false;
        }
        
        // Read number of faces
        uint16_t faceCount;
        if (!Read(faceCount)) {
            return false;
        }
        
        // Read face indices directly into material group and assign to faces
        std::vector<uint16_t> faceIndices;
        faceIndices.reserve(faceCount);
        for (uint16_t i = 0; i < faceCount; ++i) {
            uint16_t faceIndex;
            if (!Read(faceIndex)) {
                return false;
            }
            faceIndices.push_back(faceIndex);
            
            // Assign material to individual face immediately
            if (faceIndex < targetMesh_->faces.size()) {
                targetMesh_->faces[faceIndex].material = targetMaterial_;
            }
        }
        
        // Store material group in mesh
        targetMesh_->materialGroups[targetMaterial_] = std::move(faceIndices);
        
        logging::log << "Material Group: " << materialName << " with " << faceCount << " faces" << std::endl;
        return true;
    }

} // namespace Debugger3DS