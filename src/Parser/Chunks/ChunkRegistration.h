#pragma once

#include <istream>

namespace Debugger3DS::Parser::Chunks {
    // ChunkFactory is in the same namespace

    /// @brief Register every known chunk type with @p factory.
    ///
    /// Called once during `Importer::Import3DS()` before the chunk tree is
    /// walked.  Each registered chunk ID maps to a factory lambda that
    /// allocates the corresponding `Chunk` subclass.
    ///
    /// @param factory  The factory to populate.
    /// @param stream   The binary input stream shared by all chunks.
    void RegisterAllChunks(ChunkFactory& factory, std::istream& stream);

} // namespace Debugger3DS::Parser::Chunks
