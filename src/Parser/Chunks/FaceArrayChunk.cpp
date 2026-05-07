#include "FaceArrayChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
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
        
        // Bulk read interleaved face data: [a, b, c, flags] x count
        std::vector<uint16_t> raw(static_cast<size_t>(count) * 4);
        stream_.read(reinterpret_cast<char*>(raw.data()),
                     static_cast<std::streamsize>(raw.size()) * sizeof(uint16_t));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        // Deinterleave into separate arrays
        targetMesh_->faceIndices.resize(static_cast<size_t>(count) * 3);
        targetMesh_->faceMaterialIndices.resize(count, 0xFF);  // 0xFF = no material
        for (uint16_t i = 0; i < count; ++i) {
            size_t src = static_cast<size_t>(i) * 4;
            size_t dst = static_cast<size_t>(i) * 3;
            targetMesh_->faceIndices[dst]     = raw[src];
            targetMesh_->faceIndices[dst + 1] = raw[src + 1];
            targetMesh_->faceIndices[dst + 2] = raw[src + 2];
        }
        
        logging::log << "Face Array: " << count << " faces" << std::endl;
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks