#pragma once

#include "Chunk.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS::Scene { class Mesh; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;

/// @brief Point array chunk (0x4110) — reads the vertex position list for the current mesh.
///
/// @details Required; appears exactly once as a child of N_TRI_OBJECT (0x4100).
/// Vertex count must be in the range [1, 65535]. Coordinates are stored in global
/// (world) space.
///
/// @par Binary layout
/// - `uint16_t count` — number of vertices (1–65535).
/// - `count × { float x, float y, float z }` — vertex positions in global coordinates.
class PointArrayChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh> targetMesh_ = nullptr; ///< Mesh being populated.
};

} // namespace Debugger3DS::Parser::Chunks