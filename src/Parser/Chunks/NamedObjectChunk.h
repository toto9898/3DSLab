#pragma once

#include "Chunk.h"
#include <string>
#include <memory>

namespace Debugger3DS::Scene { class NamedObject; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::NamedObject;

/// @brief Named object chunk (0x4000) — introduces a named scene object (mesh, light, or camera).
///
/// @details Appears as a child of MDATA (0x3D3D). Can appear as many times as
/// needed, but each object name must be unique across the entire file.
/// Names must appear in the file before any keyframe node tag references them.
///
/// @par Binary layout
/// - `string name` — NUL-terminated, 1–10 characters + NUL terminator.
///
/// @par Expected children (exactly one object type required)
/// - N_TRI_OBJECT (0x4100) — mesh object.
/// - N_DIRECT_LIGHT (0x4600) — light object.
/// - N_CAMERA (0x4700) — camera object.
///
/// @par Optional flag children
/// OBJ_HIDDEN (0x4010), OBJ_VIS_LOFTER (0x4011), OBJ_DOESNT_CAST (0x4012),
/// OBJ_MATTE (0x4013), OBJ_FAST (0x4014), OBJ_PROCEDURAL (0x4015), OBJ_FROZEN (0x4016).
class NamedObjectChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer)  override;
    bool Process(Importer& importer)   override;

    /// @brief Return the parsed object name.
    const std::string& GetObjectName() const;

private:
    std::string name_;                              ///< Null-terminated object name from the file.
    std::shared_ptr<NamedObject> targetObject_ = nullptr; ///< Resolved scene object (unused post-parse).
};

} // namespace Debugger3DS::Parser::Chunks