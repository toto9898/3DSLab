#pragma once

#include "Chunk.h"

namespace Debugger3DS {
    
    // Main 3DS file chunk - contains other chunks
    class M3dMagicChunk : public Chunk {
        using Chunk::Chunk;  // Inherit constructor
    };
    
    // Main data section chunk - contains other chunks
    class MDataChunk : public Chunk {
        using Chunk::Chunk;  // Inherit constructor
    };
    
    // Keyframe data chunk - contains other chunks
    class KfDataChunk : public Chunk {
        using Chunk::Chunk;  // Inherit constructor
    };
    
    // Material entry chunk - contains other chunks
    class MatEntryChunk : public Chunk {
        using Chunk::Chunk;  // Inherit constructor
    };

    // AMBIENT_LIGHT (0x2100) — container: children supply the ambient color via COLOR_F/COLOR_24
    class AmbientLightChunk : public Chunk {
        using Chunk::Chunk;
    };

} // namespace Debugger3DS