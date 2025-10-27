#include "PointArrayChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {
    
    bool PointArrayChunk::ReadData(Importer& importer) {
        uint16_t count;
        if (!ReadUShort(count)) {
            return false;
        }
        
        // Get reference to current mesh being built
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Read vertices directly into target mesh
        targetMesh_->vertices.reserve(count);
        for (uint16_t i = 0; i < count; ++i) {
            Eigen::Vector3f vertex;
            if (!ReadFloat(vertex.x()) || 
                !ReadFloat(vertex.y()) || 
                !ReadFloat(vertex.z())) {
                return false;
            }
            targetMesh_->vertices.push_back(vertex);
        }
        
        logging::log << "Point Array: " << count << " vertices" << std::endl;
        return true;
    }

} // namespace Debugger3DS