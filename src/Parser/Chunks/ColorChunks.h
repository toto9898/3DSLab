#pragma once

#include "Chunk.h"
#include <memory>
#include <type_traits>
#include <Eigen/Dense>

namespace Debugger3DS::Parser::Chunks {

/// @brief Unified color chunk template — reads RGB and writes the current material's color property.
///
/// @details Dispatches the parsed color to the correct material field (ambient,
/// diffuse, specular, or scene ambient light) by inspecting the parent chunk's
/// type ID at read time.
///
/// @par Supported wire formats
/// - `T = uint8_t` → **COLOR_24 (0x0011)** — three unsigned bytes (0–255), normalised to
///   0.0–1.0 before storing. Used by Releases 1 and 2; R3 writes this chunk for
///   backward compatibility alongside LIN_COLOR_24 (0x0012).
/// - `T = float`   → **COLOR_F (0x0010)** — three floats (0.0–1.0), gamma-corrected.
///   Used by Releases 1 and 2; R3 prefers LIN_COLOR_F (0x0013) but may also
///   write COLOR_F for backward compatibility.
///
/// @note LIN_COLOR_24 (0x0012) and LIN_COLOR_F (0x0013) are the linearised
/// (non-gamma-corrected) equivalents introduced in Release 3. This implementation
/// aliases them to the same chunk types as COLOR_24 and COLOR_F respectively.
template<typename T>
class ColorChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
};

/// @brief 24-bit integer color chunk (0x0011).
using Color24Chunk = ColorChunk<uint8_t>;
/// @brief Float color chunk (0x0010).
using ColorFChunk  = ColorChunk<float>;

} // namespace Debugger3DS::Parser::Chunks
