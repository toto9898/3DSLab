#include "SmoothGroupChunk.h"
#include "../Importer.h"
#include "../SceneObjects/Mesh.h"
#include <iostream>

namespace Debugger3DS {

    bool SmoothGroupChunk::ReadData(Importer& importer) {
        // Get the current mesh being processed
        std::shared_ptr<Debugger3DS::Mesh> currentMesh = importer.GetCurrentMesh();
        if (!currentMesh) {
            logging::log << "Warning: SMOOTH_GROUP chunk found but no current mesh" << std::endl;
            return false;
        }
        
        // Get the number of faces from the current mesh
        size_t faceCount = currentMesh->GetFaceCount();
        if (faceCount == 0) {
            logging::log << "Warning: SMOOTH_GROUP chunk found but mesh has no faces" << std::endl;
            return false;
        }
        
        // Read smoothing group data for each face
        smoothingGroups_.resize(faceCount);
        
        for (size_t i = 0; i < faceCount; ++i) {
            if (!ReadULong(smoothingGroups_[i])) {
                logging::log << "Error: Failed to read smoothing group " << i << std::endl;
                return false;
            }
        }
        
        // Apply smoothing groups to the current mesh
        currentMesh->SetSmoothingGroups(smoothingGroups_);
        
        logging::log << "Smoothing Groups: " << faceCount << " face smoothing assignments" << std::endl;

        if (faceCount == 2624) {
            logging::log << "Special case: 2624 face smoothing assignments detected" << std::endl;
        }

        return true;
    }
    
    std::string SmoothGroupChunk::GetInfo() const {
        return "Smoothing Groups: " + std::to_string(smoothingGroups_.size()) + " face assignments";
    }

} // namespace Debugger3DS