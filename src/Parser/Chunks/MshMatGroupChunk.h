#pragma once

#include "Chunk.h"
#include <vector>
#include <string>
#include <memory>

namespace Debugger3DS::Scene { class Mesh; struct Material; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;
using Debugger3DS::Scene::Material;

/// @brief Mesh material-group chunk (0x4130) — assigns a named material to a subset of faces.
///
/// @details Optional; may appear multiple times inside FACE_ARRAY (0x4120) —
/// once per material applied to the mesh. The named material must have been
/// defined by a prior MAT_ENTRY / MAT_NAME chunk in the MDATA section.
///
/// When `count` is 0 the face-index list is absent (this indicates a box-mapped
/// material where no explicit face list is needed).
///
/// @par Binary layout
/// - `string name` — NUL-terminated material name (1–16 characters + NUL).
/// - `uint16_t count` — number of assigned face indices.
/// - `count × uint16_t faceIndex` — indices into the FACE_ARRAY face list.
///   No index may exceed the FACE_ARRAY face count.
class MshMatGroupChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh>     targetMesh_     = nullptr; ///< Mesh whose material groups are updated.
    std::shared_ptr<Material> targetMaterial_ = nullptr; ///< Material being assigned to the face list.
};

} // namespace Debugger3DS::Parser::Chunks