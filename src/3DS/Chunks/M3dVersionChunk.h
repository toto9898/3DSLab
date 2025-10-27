#pragma once

#include "Chunk.h"

namespace Debugger3DS {
    
    // Version chunk
    class M3dVersionChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        uint32_t GetVersion() const;
        
        // Override for custom display info
        std::string GetInfo() const override;
        std::string GetTypeName() const override;
        
    private:
        uint32_t version_;
    };
    
} // namespace Debugger3DS