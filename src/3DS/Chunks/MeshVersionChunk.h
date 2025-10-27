#pragma once

#include "Chunk.h"

namespace Debugger3DS {
    
    // Mesh version chunk
    class MeshVersionChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        uint32_t GetVersion() const;
        
    private:
        uint32_t version_ = 0;
    };
    
} // namespace Debugger3DS