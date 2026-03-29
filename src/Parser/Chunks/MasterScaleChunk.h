#pragma once

#include "Chunk.h"

namespace Debugger3DS {
    
    // Master scale chunk
    class MasterScaleChunk : public Chunk {
    public:     
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        float GetScale() const;

    private:
        float scale_;
    };
    
} // namespace Debugger3DS