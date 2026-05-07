#include "TexVertsChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
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
        
        // Bulk read all texture coordinates in one I/O call
        targetMesh_->texCoords.resize(count);
        stream_.read(reinterpret_cast<char*>(targetMesh_->texCoords.data()),
                     static_cast<std::streamsize>(count) * sizeof(Eigen::Vector2f));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        // Flip V coordinate (3DS uses bottom-left origin)
        for (auto& tc : targetMesh_->texCoords) {
            tc.y() = 1.0f - tc.y();
        }
        
        logging::log << "Texture Vertices: " << count << " texture coordinates" << std::endl;
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks