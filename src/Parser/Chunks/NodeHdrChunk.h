#pragma once

#include "Chunk.h"
#include <Eigen/Dense>
#include <memory>

namespace Debugger3DS::Scene { class NamedObject; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::NamedObject;

/// @brief Node header chunk (0xB010) — links an animation node to its scene object and hierarchy parent.
///
/// @details Required; appears exactly once inside each node tag chunk
/// (OBJECT_NODE_TAG, CAMERA_NODE_TAG, LIGHT_NODE_TAG, etc.).
///
/// @par Binary layout
/// - `string name` — NUL-terminated object name (1–10 characters + NUL).
///   A name of `"$$$DUMMY"` creates a special dummy OBJECT_NODE_TAG with no
///   associated scene object.
/// - `uint16_t flags1` — node display flags:
///   - Bit 2 (0x0004) — render: hide object.
///   - Bit 3 (0x0008) — light is off in Keyframer.
///   - Bit 11 (0x0800) — hide node in Keyframer.
///   - Bit 12 (0x1000) — fast-draw node in Keyframer.
///   - Bit 14 (0x4000) — node tag is NOT an instance.
/// - `uint16_t flags2` — additional node flags:
///   - Bit 1 (0x0001) — display motion path.
///   - Bit 2 (0x0002) — node uses auto-smoothing.
///   - Bit 3 (0x0004) — node is frozen.
///   - Bit 5 (0x0010) — node uses motion blur.
///   - Bit 6 (0x0020) — node has material morphing.
///   - Bit 7 (0x0040) — node has mesh morphing.
/// - `uint16_t parentId` — parent node index; `0xFFFF` (65535) means no parent (root level).
///   The index is based on the order nodes appear in the KFDATA section, which can
///   be overridden by NODE_ID chunks (Release 3).
class NodeHdrChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

    const std::string& GetName()        const; ///< Mesh/object name this node references.
    uint16_t           GetFlags1()      const; ///< Node flags field 1.
    uint16_t           GetFlags2()      const; ///< Node flags field 2.
    uint16_t           GetParentIndex() const; ///< Parent node ID (0xFFFF = root).

private:
    std::string name_;                               ///< Associated mesh/object name.
    uint16_t flags1_   = 0;                          ///< Flags field 1.
    uint16_t flags2_   = 0;                          ///< Flags field 2.
    uint16_t parentId_ = 0;                          ///< Parent node index.
    std::shared_ptr<NamedObject> targetObject_ = nullptr; ///< Resolved scene object (unused post-parse).
};

} // namespace Debugger3DS::Parser::Chunks