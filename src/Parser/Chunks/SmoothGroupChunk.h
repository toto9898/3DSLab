#pragma once

#include "Chunk.h"
#include <vector>
#include <cstdint>

namespace Debugger3DS {

    // Smooth Group Chunk (0x4150)
    // Contains smoothing group assignments for faces in a mesh
    // Each face can be assigned to multiple smoothing groups (bitfield)
    class SmoothGroupChunk : public Chunk {
    private:
        std::vector<uint32_t> smoothingGroups_;
        
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        // Read smoothing group data from stream
        bool ReadData(Importer& importer) override;
        
        // Get smoothing groups
        const std::vector<uint32_t>& GetSmoothingGroups() const { return smoothingGroups_; }
        
        // Get chunk type name for debugging
        std::string GetTypeName() const override { return "SMOOTH_GROUP"; }
        
        // Get chunk info
        std::string GetInfo() const override;
    };

} // namespace Debugger3DS