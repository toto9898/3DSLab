#pragma once

#include "Chunk.h"
#include <vector>
#include <cstdint>

namespace Debugger3DS::Parser::Chunks {

/// @brief Smooth group chunk (0x4150) — per-face smoothing-group bitmask array.
///
/// @details Optional; appears at most once as a child of FACE_ARRAY (0x4120).
/// The list must contain exactly one `uint32_t` entry per face in FACE_ARRAY.
///
/// Each bit in the bitmask (bits 0–31) represents one smoothing group.
/// Two adjacent faces with at least one matching set bit will have their shared
/// edge's normals smoothed. A face can belong to only one group at a time
/// (only one bit should be set per face).
///
/// @par Binary layout
/// - `count × uint32_t flags` — one bitmask per face (count = FACE_ARRAY face count;
///   no explicit count field; inferred from the chunk's data length).
class SmoothGroupChunk : public Chunk {
private:
    std::vector<uint32_t> smoothingGroups_; ///< One bitmask per face.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

    /// @brief Return the smoothing-group bitmask array (one entry per face).
    const std::vector<uint32_t>& GetSmoothingGroups() const { return smoothingGroups_; }

    std::string GetTypeName() const override { return "SMOOTH_GROUP"; }
    std::string GetInfo()     const override;
};

} // namespace Debugger3DS::Parser::Chunks