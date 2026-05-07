#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief Master scale chunk (0x0100) — scene-wide unit scale factor for all geometry.
///
/// @details Required; appears exactly once as a child of MDATA (0x3D3D).
/// Total chunk size is 10 bytes (6-byte header + 4-byte payload).
///
/// All vertex coordinates stored in POINT_ARRAY chunks should be multiplied by
/// this value to convert them to real-world units. A scale of `1.0` corresponds
/// to one inch per unit.
///
/// @par Binary layout
/// - `float scale` — master scale factor (e.g. `1.0` = 1 inch/unit).
///
/// @note Only one MASTER_SCALE chunk may appear per MDATA section.
class MasterScaleChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

    /// @brief Return the parsed scale factor.
    float GetScale() const;

private:
    float scale_ = 1.0f; ///< Scene master scale factor.
};

} // namespace Debugger3DS::Parser::Chunks