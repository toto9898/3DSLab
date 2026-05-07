#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS::Scene { class Mesh; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Mesh;

/// @brief Triangle mesh object chunk (0x4100) — marks a NAMED_OBJECT as a mesh and creates a `Mesh`.
///
/// @details Appears as a child of NAMED_OBJECT (0x4000); at most one per NAMED_OBJECT
/// and mutually exclusive with N_DIRECT_LIGHT and N_CAMERA. Contains no direct
/// binary data — all geometry is provided by child chunks.
///
/// @par Required children
/// - POINT_ARRAY (0x4110) — vertex positions.
/// - FACE_ARRAY (0x4120) — triangle indices and edge flags.
///
/// @par Optional children
/// - POINT_FLAG_ARRAY (0x4111) — per-vertex flags.
/// - TEX_VERTS (0x4140) — UV coordinates (one pair per vertex).
/// - MESH_MATRIX (0x4160) — local-to-world transform (3×4 matrix).
/// - MSH_MAT_GROUP (0x4130) — material–face assignments (one chunk per material).
/// - SMOOTH_GROUP (0x4150) — per-face smoothing group bitmasks.
/// - MESH_COLOR (0x4165) — UI wireframe color index (R3).
/// - MESH_TEXTURE_INFO (0x4170) — mapping icon orientation (not used by renderer).
/// - MSH_BOXMAP (0x4190) — box-mapping material list (R3).
/// - PROC_NAME (0x4181) — animated stand-in procedure name (R2/R3).
/// - PROC_DATA (0x4182) — animated procedural data (R2/R3).
class NTriObjectChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

private:
    std::shared_ptr<Mesh> mesh_ = nullptr; ///< Newly-created mesh, shared with the importer.
};

// ============================================================================
/// @name Optional N_TRI_OBJECT child chunks
/// @{
// ============================================================================

/// @brief Point flag array chunk (0x4111) — per-vertex flag list for the current mesh.
///
/// @details Optional; appears at most once per N_TRI_OBJECT. The flag count must
/// equal the POINT_ARRAY vertex count. Flags have internal 3D Studio meaning.
///
/// @par Binary layout
/// - `uint16_t count` — number of vertex flags (must equal vertex count).
/// - `count × int16_t flags` — one flag per vertex.
class PointFlagArrayChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "POINT_FLAG_ARRAY"; }
};

/// @brief Mesh color chunk (0x4165) — UI wireframe color index for this mesh.
///
/// @details Optional (Release 3); appears at most once per N_TRI_OBJECT.
/// Determines the color used when drawing the mesh wireframe inside 3D Studio's
/// interface. Total chunk size is 7 bytes.
///
/// @par Binary layout
/// - `uint8_t colorIndex` — color table index (0–64).
class MeshColorChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "MESH_COLOR"; }
};

/// @brief Mesh texture info chunk (0x4170) — orientation of the last-used mapping icon.
///
/// @details Optional; appears at most once per N_TRI_OBJECT. Records the position
/// and orientation of the mapping icon as it was when UV coordinates were last
/// generated inside 3D Studio. This data is informational only and is NOT used
/// by the renderer to determine texture placement. Total chunk size is 90 bytes.
///
/// @par Binary layout
/// - `float  xTiling`    — X tiling repeat value.
/// - `float  yTiling`    — Y tiling repeat value.
/// - `float[3] position` — icon position in global coordinates.
/// - `float  scale`      — icon uniform scale in world units.
/// - `float[12] matrix`  — 3×4 orientation matrix for the icon.
/// - `float  width`      — planar icon width.
/// - `float  height`     — planar icon height.
/// - `float  cylHeight`  — cylindrical icon height.
class MeshTextureInfoChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "MESH_TEXTURE_INFO"; }
};

/// @brief Box mapping material list chunk (0x4190) — six material names for box mapping.
///
/// @details Optional (Release 3); appears at most once per FACE_ARRAY (0x4120).
/// Specifies one material name per box face (front, back, left, right, top, bottom).
/// Each string is NUL-terminated (1–16 characters + NUL).
///
/// @par Binary layout
/// Six NUL-terminated strings in order: front, back, left, right, top, bottom.
/// Each is 1–16 characters + NUL terminator.
class MshBoxmapChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "MSH_BOXMAP"; }
};

/// @brief Procedural name chunk (0x4181) — name of the animated stand-in procedure.
///
/// @details Optional (Release 2/3); appears at most once per N_TRI_OBJECT, and only
/// when OBJ_PROCEDURAL (0x4015) is defined in the parent NAMED_OBJECT.
///
/// @par Binary layout
/// - `string name` — NUL-terminated, up to 12 characters + NUL (.axp extension expected).
class ProcNameChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "PROC_NAME"; }
};

/// @brief Procedural data chunk (0x4182) — state data for the animated stand-in procedure.
///
/// @details Optional (Release 2/3); appears at most once per N_TRI_OBJECT, and only
/// when PROC_NAME (0x4181) refers to an animated procedure (.axp extension).
/// Content format depends on the procedure's state structure.
class ProcDataChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "PROC_DATA"; }
};

/// @}

} // namespace Debugger3DS::Parser::Chunks