#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief Camera chunk (0x4700) — defines a camera object inside a NAMED_OBJECT.
///
/// @details Appears at most once per NAMED_OBJECT (0x4000), mutually exclusive
/// with N_TRI_OBJECT and N_DIRECT_LIGHT. Reads all camera parameters inline and
/// creates a `Camera` entry in the scene.
///
/// @par Binary layout
/// - `float[3] position`   — camera position in global coordinates.
/// - `float[3] target`     — camera look-at target in global coordinates.
/// - `float    bankAngle`  — camera roll/bank angle in degrees.
/// - `float    focalLength` — focal length in millimeters.
///
/// @par Optional children
/// - CAM_SEE_CONE (0x4710) — flag: show camera cone in viewport (R2 only).
/// - CAM_RANGES (0x4720) — near/far atmospheric effect range (R3).
class NCameraChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "N_CAMERA"; }
};

/// @brief Camera atmospheric ranges chunk (0x4720) — near/far effect radii for the current camera.
///
/// @details Optional (Release 3); appears at most once per N_CAMERA. Defines
/// the near and far distances used by atmospheric effects (fog, distance cue)
/// associated with this camera's view. Total chunk size: 14 bytes.
///
/// @par Binary layout
/// - `float near` — near effect radius (≥ 0).
/// - `float far`  — far effect radius (≥ 0; should be ≥ near).
class CamRangesChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "CAM_RANGES"; }
};

/// @brief Camera cone visibility flag chunk (0x4710) — shows the camera's view cone in the viewport.
///
/// @details Optional (Release 2 only); appears at most once per N_CAMERA (0x4700).
/// No binary data payload (6-byte header only). Only present in files created
/// in Release 2.
class CamSeeConeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "CAM_SEE_CONE"; }
};

} // namespace Debugger3DS::Parser::Chunks
