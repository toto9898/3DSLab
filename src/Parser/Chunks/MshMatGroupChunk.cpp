#include "MshMatGroupChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool MshMatGroupChunk::ReadData(Importer& importer) {
        // Read material name
        std::string materialName;
        if (!Read(materialName)) {
            return false;
        }
        
        // Get current mesh
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Look up the material by name from the scene
        for (const auto& mat : importer.GetScene().materials) {
            if (mat->name == materialName) {
                targetMaterial_ = mat;
                break;
            }
        }
        
        if (!targetMaterial_) {
            logging::log << "Material Group: material '" << materialName << "' not found" << std::endl;
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