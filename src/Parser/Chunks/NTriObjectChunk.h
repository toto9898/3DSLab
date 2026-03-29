#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS {
    
    // Forward declaration
    class Mesh;
    
    // Mesh object chunk - contains other chunks
    class NTriObjectChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;

    private:
        std::shared_ptr<Mesh> mesh_ = nullptr;
    };
    
} // namespace Debugger3DS