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
        
        // Bulk read all face indices in one I/O call
        std::vector<uint16_t> faceIndices(faceCount);
        stream_.read(reinterpret_cast<char*>(faceIndices.data()),
                     static_cast<std::streamsize>(faceCount) * sizeof(uint16_t));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        // Assign material to individual faces
        for (uint16_t faceIndex : faceIndices) {
            if (faceIndex < targetMesh_->faceMaterials.size()) {
                targetMesh_->faceMaterials[faceIndex] = targetMaterial_;
            }
        }
        
        // Store material group in mesh
        targetMesh_->materialGroups[targetMaterial_] = std::move(faceIndices);
        
        logging::log << "Material Group: " << materialName << " with " << faceCount << " faces" << std::endl;
        return true;
    }

} // namespace Debugger3DS