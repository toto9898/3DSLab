#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief Mesh section version chunk (0x3D3E) — stores the MDATA format revision number.
///
/// @details Required; appears exactly once as a direct child of MDATA (0x3D3D).
/// Total chunk size is 10 bytes (6-byte header + 4-byte payload).
///
/// @par Binary layout
/// - `uint32_t version` — mesh revision level.
///   - 1 = 3D Studio Release 1
///   - 2 = 3D Studio Release 2
///   - 3 = 3D Studio Release 3 (most common)
///
/// @note Only one MESH_VERSION chunk may appear per MDATA section.
class MeshVersionChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

    /// @brief Return the parsed mesh version number.
    uint32_t GetVersion() const;

private:
    uint32_t version_ = 0; ///< Mesh format version.
};

} // namespace Debugger3DS::Parser::Chunks