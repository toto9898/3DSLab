#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief Direct light chunk (0x4600) — defines an omni- or spotlight object inside a NAMED_OBJECT.
///
/// @details Appears at most once per NAMED_OBJECT (0x4000), mutually exclusive
/// with N_TRI_OBJECT and N_CAMERA. Reads the light's world-space position and
/// creates a `Light` entry in the scene. The light color is provided by a child
/// COLOR_F (0x0010) or COLOR_24 (0x0011) chunk.
///
/// @par Binary layout
/// - `float[3] position` — light position in global coordinates.
///
/// @par Optional children
/// - COLOR_F (0x0010) / COLOR_24 (0x0011) — light color.
/// - DL_SPOTLIGHT (0x4610) — converts the omni-light to a spotlight.
/// - DL_OFF (0x4620) — flag chunk; light is disabled.
/// - DL_INNER_RANGE (0x4659) — inner attenuation radius (R3).
/// - DL_OUTER_RANGE (0x465A) — outer attenuation radius (R3).
/// - DL_MULTIPLIER (0x465B) — intensity multiplier (R3).
/// - DL_ATTENUATE (0x4625) — enable attenuation flag (R3).
class NDirectLightChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "N_DIRECT_LIGHT"; }
};

/// @brief Spotlight sub-chunk (0x4610) — converts the current omni-light into a spotlight.
///
/// @details Optional; appears at most once as a child of N_DIRECT_LIGHT (0x4600).
/// When present, the parent light becomes a spotlight aimed at the specified target.
///
/// @par Binary layout
/// - `float[3] target` — spotlight target position in global coordinates.
/// - `float    hotspot` — inner (hotspot) cone half-angle in degrees (1–160).
/// - `float    falloff` — outer (falloff) cone half-angle in degrees (1–160; ≥ hotspot).
///
/// @par Optional children
/// DL_SPOT_ROLL (0x4656), DL_SHADOWED (0x4630), DL_LOCAL_SHADOW2 (0x4641),
/// DL_SEE_CONE (0x4650), DL_SPOT_RECTANGULAR (0x4651), DL_SPOT_OVERSHOOT (0x4652),
/// DL_SPOT_PROJECTOR (0x4653), DL_SPOT_ASPECT (0x4657), DL_RAY_BIAS (0x4658),
/// DL_RAYSHAD (0x4627).
class DLSpotlightChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "DL_SPOTLIGHT"; }
};

/// @brief Light-off flag chunk (0x4620) — marks the current light as disabled.
///
/// @details Optional; appears at most once per N_DIRECT_LIGHT. No binary data
/// payload (6-byte header only). The light object is created but does not
/// illuminate the scene.
class DLOffChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "DL_OFF"; }
};

/// @brief Inner attenuation range chunk (0x4659) — near attenuation radius for the current light.
///
/// @details Optional (Release 3); appears at most once per N_DIRECT_LIGHT.
/// Within this radius the light has full intensity. Total chunk size: 10 bytes.
///
/// @par Binary layout
/// - `float radius` — inner attenuation radius in world units (≥ 0).
class DLInnerRangeChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "DL_INNER_RANGE"; }
};

/// @brief Outer attenuation range chunk (0x465A) — far attenuation radius for the current light.
///
/// @details Optional (Release 3); appears at most once per N_DIRECT_LIGHT.
/// At this radius the light intensity falls to zero. Total chunk size: 10 bytes.
///
/// @par Binary layout
/// - `float radius` — outer attenuation radius in world units (≥ 0; should be ≥ DL_INNER_RANGE).
class DLOuterRangeChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "DL_OUTER_RANGE"; }
};

/// @brief Light multiplier chunk (0x465B) — intensity multiplier that scales the light color.
///
/// @details Optional (Release 3); appears at most once per N_DIRECT_LIGHT.
/// Total chunk size: 10 bytes. A value of 1.0 leaves the color unchanged.
/// Negative values produce a darkening (subtractive) effect.
///
/// @par Binary layout
/// - `float multiplier` — intensity multiplier (any non-zero value).
class DLMultiplierChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "DL_MULTIPLIER"; }
};

/// @brief Light attenuation flag chunk (0x4625) — enables distance attenuation for the current light.
///
/// @details Optional (Release 3); appears at most once per N_DIRECT_LIGHT.
/// No binary data payload (6-byte header only). When present, the inner/outer
/// attenuation radii defined by DL_INNER_RANGE and DL_OUTER_RANGE take effect.
class DLAttenuateChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_ATTENUATE"; }
};

/// @brief Ray-traced shadows flag chunk (0x4627) — enables ray-traced shadow casting for the current spotlight.
///
/// @details Optional; appears at most once per DL_SPOTLIGHT (0x4610).
/// No binary data payload (6-byte header only). Found in files created in Release 3.
class DLRayshadChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_RAYSHAD"; }
};

/// @brief Spotlight shadowed flag chunk (0x4630) — enables shadow casting for the current spotlight.
///
/// @details Optional; appears at most once per DL_SPOTLIGHT (0x4610).
/// No binary data payload (6-byte header only).
class DLShadowedChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SHADOWED"; }
};

/// @brief Local shadow parameters chunk (0x4641) — per-spotlight shadow quality settings.
///
/// @details Optional; appears at most once per DL_SPOTLIGHT (0x4610).
/// Found in files from Release 2 and 3. Total chunk size is 16 bytes.
///
/// @par Binary layout
/// - `float  bias`   — local shadow low bias.
/// - `float  filter` — local shadow filter (range: 1 lowest to 10 highest).
/// - `int16_t size`  — local shadow map size (range: 10–4096).
class DLLocalShadow2Chunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_LOCAL_SHADOW2"; }
};

/// @brief Spotlight cone visibility flag chunk (0x4650) — shows the spotlight cone in the viewport.
///
/// @details Optional; appears at most once per DL_SPOTLIGHT (0x4610).
/// No binary data payload (6-byte header only). Found in files from Release 2 and 3.
class DLSeeConeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SEE_CONE"; }
};

/// @brief Rectangular spotlight cone flag chunk (0x4651) — makes the spotlight cone rectangular.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// No binary data payload (6-byte header only). When present, the spotlight
/// projects a rectangular beam instead of a circular cone.
class DLSpotRectangularChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SPOT_RECTANGULAR"; }
};

/// @brief Spotlight overshoot flag chunk (0x4652) — enables spotlight overshoot.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// No binary data payload (6-byte header only). When present, the spotlight
/// illuminates objects outside the cone's boundary.
class DLSpotOvershootChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SPOT_OVERSHOOT"; }
};

/// @brief Spotlight projector bitmap chunk (0x4653) — projects a bitmap through the spotlight.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// The named bitmap is projected through the spotlight cone.
///
/// @par Binary layout
/// - `string filename` — NUL-terminated, 1–12 characters + NUL terminator.
///   May contain extra padding characters after the NUL.
class DLSpotProjectorChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SPOT_PROJECTOR"; }
};

/// @brief Light exclusion chunk (0x4654) — excludes a named object from this light's illumination.
///
/// @details Optional (Release 3); may appear multiple times per N_DIRECT_LIGHT —
/// once per excluded object. Total length depends on the string length.
///
/// @par Binary layout
/// - `string name` — NUL-terminated name of an N_TRI_OBJECT to exclude (1–10 chars + NUL).
class DLExcludeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_EXCLUDE"; }
};

/// @brief Spotlight roll angle chunk (0x4656) — bank/roll angle of the spotlight cone.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float angle` — roll angle in degrees (range: −360.0 to +360.0).
class DLSpotRollChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SPOT_ROLL"; }
};

/// @brief Spotlight aspect ratio chunk (0x4657) — width/height ratio for a rectangular spotlight cone.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// Only meaningful when DL_SPOT_RECTANGULAR is also present. Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float ratio` — width-to-height ratio of the spotlight cone (must not be 0.0).
class DLSpotAspectChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_SPOT_ASPECT"; }
};

/// @brief Ray-traced shadow bias chunk (0x4658) — per-spotlight ray-traced shadow bias value.
///
/// @details Optional (Release 3); appears at most once per DL_SPOTLIGHT (0x4610).
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float bias` — ray-traced shadow bias value.
class DLRayBiasChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DL_RAY_BIAS"; }
};

} // namespace Debugger3DS::Parser::Chunks
