#include "PointArrayChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    bool PointArrayChunk::ReadData(Importer& importer) {
        uint16_t count;
        if (!Read(count)) {
            return false;
        }
        
        // Get reference to current mesh being built
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Bulk read all vertices in one I/O call
        // Safe: Eigen::Vector3f is 3 contiguous floats, 3DS is little-endian matching x86
        targetMesh_->vertices.resize(count);
        stream_.read(reinterpret_cast<char*>(targetMesh_->vertices.data()),
                     static_cast<std::streamsize>(count) * sizeof(Eigen::Vector3f));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        logging::log << "Point Array: " << count << " vertices" << std::endl;
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks