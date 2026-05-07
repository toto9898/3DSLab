#pragma once

#include "Chunk.h"
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS::Scene { class Mesh; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;

/// @brief Mesh matrix chunk (0x4160) — local-to-world transform for the current mesh.
///
/// @details Optional; appears at most once as a child of N_TRI_OBJECT (0x4100).
/// Total chunk size is 54 bytes (6-byte header + 48-byte matrix).
///
/// Stores the object's local coordinate axes and translation in global space.
/// The Keyframer uses this to determine the initial object orientation and the
/// center of rotation. Vertex positions in POINT_ARRAY are in global space;
/// the inverse of this matrix converts them to local (object) space.
///
/// @par Binary layout
/// - `float[12]` — 3×4 row-major matrix:
///   - Rows 0–2: local X, Y, Z axis unit vectors (3 floats each).
///   - Implicit 4th column is the translation vector (last 3 floats).
///
/// This implementation reads the 12 values and expands them into an
/// `Eigen::Matrix4f` with the 4th row set to `[0, 0, 0, 1]`.
class MeshMatrixChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh> targetMesh_ = nullptr; ///< Mesh whose transform is being set.
};

} // namespace Debugger3DS::Parser::Chunks