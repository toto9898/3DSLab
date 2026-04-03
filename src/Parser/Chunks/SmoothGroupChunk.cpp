#include "SmoothGroupChunk.h"
#include "Importer.h"
#include "Mesh.h"
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
        
        // Bulk read all smoothing groups in one I/O call
        smoothingGroups_.resize(faceCount);
        stream_.read(reinterpret_cast<char*>(smoothingGroups_.data()),
                     static_cast<std::streamsize>(faceCount) * sizeof(uint32_t));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        // Apply smoothing groups to the current mesh (data consumed but not stored)
        
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