#pragma once

#include "Chunk.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS::Scene { class Mesh; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;

/// @brief Texture-coordinate chunk (0x4140) — reads UV pairs for the current mesh.
///
/// @details Optional; appears at most once as a child of N_TRI_OBJECT (0x4100).
/// The vertex count must equal the POINT_ARRAY vertex count (one UV pair per vertex).
/// UV values are unconstrained but are typically in the range [0, 1].
///
/// @par Binary layout
/// - `uint16_t count` — number of UV pairs (must equal POINT_ARRAY vertex count).
/// - `count × { float u, float v }` — texture coordinates.
class TexVertsChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh> targetMesh_ = nullptr; ///< Mesh being populated.
};

} // namespace Debugger3DS::Parser::Chunks