#pragma once

#include "Chunk.h"
#include <cstdint>

namespace Debugger3DS::Parser::Chunks {

/// @brief Keyframe header chunk (0xB00A) — global animation metadata for the KFDATA section.
///
/// @details Required; appears exactly once as a child of KFDATA (0xB000).
/// Total chunk size is 25 bytes (6-byte header + 2-byte revision + NUL-terminated
/// filename of up to 12 chars + 4-byte animation length).
///
/// @par Binary layout
/// - `uint16_t revision` — keyframe format revision:
///   - 1 = 3D Studio Release 1
///   - 2 = 3D Studio Release 2
///   - 5 = 3D Studio Release 3 (most common)
/// - `string filename` — NUL-terminated source file name (12 chars + NUL);
///   informational only and may contain a stale path.
/// - `uint32_t animLength` — total animation length in frames (valid range: 1–32000).
class KfHdrChunk : public Chunk {
private:
    uint16_t    revision_    = 0; ///< Animation format revision.
    std::string filename_;        ///< Original source file name (informational).
    uint32_t    animLength_  = 0; ///< Total animation length in frames.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "KFHDR"; }
    std::string GetInfo()     const override;
};

/// @brief Keyframe segment chunk (0xB008) — defines the active frame range for rendering/playback.
///
/// @details Optional; appears at most once as a child of KFDATA (0xB000).
/// Total chunk size is 14 bytes (6-byte header + 4-byte start + 4-byte end).
/// Determines which range of frames will be rendered or played back. If absent,
/// the full animation length from KFHDR is assumed.
///
/// @par Binary layout
/// - `uint32_t start` — first frame of the active segment.
/// - `uint32_t end`   — last frame of the active segment.
class KfSegChunk : public Chunk {
private:
    uint32_t start_ = 0; ///< First frame of the active segment.
    uint32_t end_   = 0; ///< Last frame of the active segment.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "KFSEG"; }
    std::string GetInfo()     const override;
};

/// @brief Keyframe current-time chunk (0xB009) — the current (active) frame in the Keyframer.
///
/// @details Optional; appears at most once as a child of KFDATA (0xB000).
/// Total chunk size is 10 bytes (6-byte header + 4-byte frame number).
/// Determines which single frame is active when the file is opened in 3D Studio's
/// Keyframer, or the frame to render in a single-frame render.
///
/// @par Binary layout
/// - `uint32_t currentFrame` — current frame number.
class KfCurTimeChunk : public Chunk {
private:
    uint32_t currentFrame_ = 0; ///< Current frame number.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "KFCURTIME"; }
    std::string GetInfo()     const override;
};

} // namespace Debugger3DS::Parser::Chunks