#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief File version chunk (0x0002) — stores the overall .3ds format revision number.
///
/// @details Required; appears exactly once as a direct child of M3DMAGIC (0x4D4D).
/// Total chunk size is 10 bytes (6-byte header + 4-byte payload).
///
/// @par Binary layout
/// - `uint32_t version` — revision number.
///   - 1 = 3D Studio Release 1
///   - 2 = 3D Studio Release 2
///   - 3 = 3D Studio Release 3 (most common)
///
/// @note Only one M3D_VERSION chunk may appear per file.
class M3dVersionChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

    /// @brief Return the parsed version number.
    uint32_t GetVersion() const;

    std::string GetInfo()     const override;
    std::string GetTypeName() const override;

private:
    uint32_t version_ = 0; ///< 3DS format version (typically 3).
};

} // namespace Debugger3DS::Parser::Chunks