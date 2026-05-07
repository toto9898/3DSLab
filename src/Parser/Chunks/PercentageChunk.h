#pragma once

#include "Chunk.h"
#include <memory>

namespace Debugger3DS::Scene { struct Material; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Material;

/// @brief Integer percentage chunk (0x0030) — reads a signed 16-bit percentage value.
///
/// @details Appears as a child of material property container chunks (shininess,
/// transparency, self-illumination, etc.) and map container chunks (blend strength).
/// Total chunk size is 8 bytes (6-byte header + 2-byte payload).
///
/// The parent chunk's type ID is used to route the value to the correct
/// `Material` property.
///
/// @par Binary layout
/// - `int16_t percentage` — value in the range 0–100 (may be negative in some
///   contexts, e.g. transparency falloff MAT_XPFALL).
class IntPercentageChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "INT_PERCENTAGE"; }

private:
    std::shared_ptr<Material> targetMaterial_; ///< Material being modified.

    /// @brief Set the correct material field based on @p parentChunkId.
    void SetMaterialProperty(int16_t percentage, uint16_t parentChunkId);
};

/// @brief Float percentage chunk (0x0031) — reads a single-precision percentage value.
///
/// @details Appears as a child of material property container chunks where
/// higher precision is needed. Total chunk size is 10 bytes (6-byte header + 4-byte payload).
///
/// The parent chunk's type ID is used to route the value to the correct
/// `Material` property.
///
/// @par Binary layout
/// - `float percentage` — value in the range 0.0–1.0 (may be negative in some contexts).
class FloatPercentageChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "FLOAT_PERCENTAGE"; }

private:
    std::shared_ptr<Material> targetMaterial_; ///< Material being modified.

    /// @brief Set the correct material field based on @p parentChunkId.
    void SetMaterialProperty(float percentage, uint16_t parentChunkId);
};

} // namespace Debugger3DS::Parser::Chunks