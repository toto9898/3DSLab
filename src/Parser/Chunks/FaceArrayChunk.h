#pragma once

#include "Chunk.h"
#include <vector>
#include <memory>
#include "Mesh.h"

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;

/// @brief Face array chunk (0x4120) — reads the triangle index list and per-face edge flags.
///
/// @details Required; appears exactly once as a child of N_TRI_OBJECT (0x4100).
/// Face count must be in the range [1, 65535]. No vertex index may be ≥ the
/// POINT_ARRAY vertex count.
///
/// @par Binary layout
/// - `uint16_t count` — number of triangles.
/// - `count × { uint16_t a, uint16_t b, uint16_t c, uint16_t flags }` — one record per face.
///
/// @par Edge visibility flags (per-face `flags` field)
/// - Bit 0 (0x01) — CA edge is visible.
/// - Bit 1 (0x02) — BC edge is visible.
/// - Bit 2 (0x04) — AB edge is visible.
/// - Bit 3 (0x08) — U-axis texture wrap.
/// - Bit 4 (0x10) — V-axis texture wrap.
///
/// @par Child chunks (embedded inside FACE_ARRAY)
/// - MSH_MAT_GROUP (0x4130) — material–face assignments; zero or more.
/// - SMOOTH_GROUP (0x4150) — per-face smoothing group bitmasks; optional.
/// - MSH_BOXMAP (0x4190) — box-mapping material list; optional, R3 only.
class FaceArrayChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh> targetMesh_ = nullptr; ///< Mesh being populated.
};

} // namespace Debugger3DS::Parser::Chunks