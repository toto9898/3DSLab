#pragma once

namespace Debugger3DS {
    class ChunkFactory;
    
    // Register all chunk types with the factory
    void RegisterAllChunks(ChunkFactory& factory, std::istream& stream);
    
} // namespace Debugger3DS
