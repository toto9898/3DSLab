#pragma once

#include "Chunk.h"

namespace Debugger3DS::Parser::Chunks {

/// @brief Root container chunk (0x4D4D) — top-level wrapper for all .3ds file data.
///
/// @details Every .3ds file begins with this chunk, whose length spans the entire
/// file. No binary data is stored directly; all content lives in child chunks.
///
/// @par Expected children
/// - M3D_VERSION (0x0002) — required, exactly once.
/// - MDATA (0x3D3D) — required, exactly once.
/// - KFDATA (0xB000) — optional, at most once.
class M3dMagicChunk : public Chunk {
    using Chunk::Chunk;
};

/// @brief Mesh data section chunk (0x3D3D) — root container for all scene geometry and materials.
///
/// @details Required once per file. Appears as a child of M3DMAGIC. Contains
/// all mesh objects, materials, and scene-wide settings. Material definitions
/// (MAT_ENTRY) must appear before any NAMED_OBJECT chunk that references them.
///
/// @par Expected children
/// - MESH_VERSION (0x3D3E) — required, mesh format version.
/// - MASTER_SCALE (0x0100) — required, scene unit scale factor.
/// - MAT_ENTRY (0xAFFF) — zero or more material definitions.
/// - NAMED_OBJECT (0x4000) — zero or more mesh/light/camera definitions.
/// - AMBIENT_LIGHT (0x2100) — optional scene ambient color.
/// - VIEWPORT_LAYOUT (0x7001) — optional viewport configuration.
/// - LO_SHADOW_BIAS (0x1400) — optional global low shadow bias.
/// - HI_SHADOW_BIAS (0x1410) — optional global high shadow bias (R1).
/// - SHADOW_MAP_SIZE (0x1420) — optional global shadow map size.
/// - SHADOW_SAMPLES (0x1430) — optional global shadow sample size (R1).
/// - SHADOW_RANGE (0x1440) — optional global shadow range (R1).
/// - SHADOW_FILTER (0x1450) — optional global shadow filter (R2/R3).
/// - RAY_BIAS (0x1460) — optional ray-traced shadow bias (R3).
/// - O_CONSTS (0x1500) — optional construction plane location (R2/R3).
/// - BIT_MAP (0x1100) — optional bitmapped background filename.
/// - USE_BIT_MAP (0x1101) — optional flag: use bitmapped background.
/// - SOLID_BGND (0x1200) — optional solid color background.
/// - USE_SOLID_BGND (0x1201) — optional flag: use solid color background.
/// - V_GRADIENT (0x1300) — optional gradient background.
/// - USE_V_GRADIENT (0x1301) — optional flag: use gradient background.
/// - FOG (0x2200) — optional atmospheric fog.
/// - USE_FOG (0x2201) — optional flag: activate fog.
/// - LAYER_FOG (0x2302) — optional layer fog (R3).
/// - USE_LAYER_FOG (0x2303) — optional flag: activate layer fog (R3).
/// - DISTANCE_CUE (0x2300) — optional distance-cue darkening.
/// - USE_DISTANCE_CUE (0x2301) — optional flag: activate distance cue.
/// - DEFAULT_VIEW (0x3000) — optional default rendering view.
class MDataChunk : public Chunk {
    using Chunk::Chunk;
};

/// @brief Keyframe data section chunk (0xB000) — root container for all animation data.
///
/// @details Optional; appears at most once per file as a child of M3DMAGIC.
/// All keyframe tracks for every scene object are nested inside this chunk.
///
/// @par Expected children
/// - KFHDR (0xB00A) — required, animation metadata.
/// - KFSEG (0xB008) — optional, active segment range.
/// - KFCURTIME (0xB009) — optional, current frame.
/// - OBJECT_NODE_TAG (0xB002), CAMERA_NODE_TAG (0xB003), TARGET_NODE_TAG (0xB004),
///   LIGHT_NODE_TAG (0xB005), SPOTLIGHT_NODE_TAG (0xB007), L_TARGET_NODE_TAG (0xB006),
///   AMBIENT_NODE_TAG (0xB001) — zero or more of each, in any order.
class KfDataChunk : public Chunk {
    using Chunk::Chunk;
};

/// @brief Material entry chunk (0xAFFF) — container for a single material definition.
///
/// @details May appear as many times as needed inside MDATA, once per material.
/// Each entry must have a unique MAT_NAME. Material definitions must appear
/// in the file before any NAMED_OBJECT that references them.
///
/// @par Required children
/// - MAT_NAME (0xA000) — material name (must be unique).
/// - MAT_SHADING (0xA100) — shading model.
///
/// @par Color children
/// - MAT_AMBIENT (0xA010) — ambient color.
/// - MAT_DIFFUSE (0xA020) — diffuse (base) color.
/// - MAT_SPECULAR (0xA030) — specular highlight color.
///
/// @par Shininess / transparency children
/// - MAT_SHININESS (0xA040) — shininess ratio.
/// - MAT_SHIN2PCT (0xA041) — shininess strength (R3).
/// - MAT_TRANSPARENCY (0xA050) — transparency ratio.
/// - MAT_XPFALL (0xA052) — transparency falloff ratio (R2/R3).
/// - MAT_USE_XPFALL (0xA240) — flag: use transparency falloff (R2/R3).
/// - MAT_REFBLUR (0xA053) — reflection blur ratio (R2/R3).
/// - MAT_USE_REFBLUR (0xA250) — flag: use reflection blur (R2/R3).
///
/// @par Property flag children (all optional)
/// - MAT_SELF_ILLUM (0xA080) — self-illumination flag (R1/R2).
/// - MAT_SELF_ILPCT (0xA084) — self-illumination percentage (R3).
/// - MAT_TWO_SIDE (0xA081) — two-sided rendering flag.
/// - MAT_DECAL (0xA082) — decal mapping flag (R1/R2).
/// - MAT_ADDITIVE (0xA083) — additive transparency flag.
/// - MAT_WIRE (0xA085) — wireframe rendering flag (R3).
/// - MAT_WIRE_SIZE (0xA087) — wireframe line width (R3).
/// - MAT_WIREABS (0xA08E) — wire size in world units flag (R3).
/// - MAT_FACEMAP (0xA088) — face-mapped textures flag (R3).
/// - MAT_PHONGSOFT (0xA08C) — Phong soften flag (R3).
///
/// @par Texture / map children (all optional)
/// - MAT_TEXMAP (0xA200) — primary diffuse texture map.
/// - MAT_TEX2MAP (0xA33A) — second texture map (R3).
/// - MAT_SPECMAP (0xA204) — specular map (R3).
/// - MAT_OPACMAP (0xA210) — opacity map.
/// - MAT_BUMPMAP (0xA230) — bump/normal map.
/// - MAT_SHINMAP (0xA33C) — shininess map (R3).
/// - MAT_SELFIMAP (0xA33D) — self-illumination map (R3).
/// - MAT_REFLMAP (0xA220) — reflection map.
/// - MAT_ACUBIC (0xA310) — automatic cubic reflection map (R2/R3).
///
/// @par Texture mask children (all optional, R3)
/// - MAT_TEXMASK (0xA33E), MAT_TEX2MASK (0xA340), MAT_OPACMASK (0xA342),
///   MAT_BUMPMASK (0xA344), MAT_SHINMASK (0xA346), MAT_SPECMASK (0xA348),
///   MAT_REFLMASK (0xA34A), MAT_SELFIMASK (0xA34C).
class MatEntryChunk : public Chunk {
    using Chunk::Chunk;
};

/// @brief Ambient light chunk (0x2100) — sets the scene-wide ambient light color.
///
/// @details Appears at most once as a child of MDATA. Contains no direct binary
/// data; the actual RGB values are provided by child color sub-chunks.
///
/// @par Expected children
/// - COLOR_F (0x0010) — floating-point RGB color (R1 and R2).
/// - LIN_COLOR_F (0x0013) — linearised RGB color (R3, preferred over COLOR_F).
class AmbientLightChunk : public Chunk {
    using Chunk::Chunk;
};

// ============================================================================
/// @name File-type magic / project-file container chunks
/// @{
// ============================================================================

/// @brief Material library magic chunk (0x3DAA) — marks a .MLI material library file.
///
/// @details This is the root chunk of a .mli material library file, analogous to
/// M3DMAGIC (0x4D4D) in a .3ds scene file. Its children are MAT_ENTRY (0xAFFF)
/// chunks describing the materials contained in the library.
class MlibMagicChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "MLIBMAGIC"; }
};

/// @brief Project file magic chunk (0xC23D) — marks a .PRJ project file.
///
/// @details This is the root chunk of a .prj 3D Studio project file. It acts as a
/// container for MDATA (0x3D3D) and KFDATA (0xB000), just like M3DMAGIC, but is
/// used for the extended project format.
class CMagicChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "CMAGIC"; }
};

/// @}

// ============================================================================
/// @name Global shadow parameter chunks (children of MDATA)
/// @{
// ============================================================================

/// @brief Low shadow bias chunk (0x1400) — global minimum bias for shadow maps.
///
/// @details Optional; appears at most once per MDATA. Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float bias` — shadow low-bias value.
class LoShadowBiasChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "LO_SHADOW_BIAS"; }
};

/// @brief High shadow bias chunk (0x1410) — global maximum bias for shadow maps.
///
/// @details Optional (Release 1); appears at most once per MDATA. Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float bias` — shadow high-bias value.
class HiShadowBiasChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "HI_SHADOW_BIAS"; }
};

/// @brief Shadow map size chunk (0x1420) — resolution of the global shadow map.
///
/// @details Optional; appears at most once per MDATA. Total chunk size is 8 bytes.
///
/// @par Binary layout
/// - `int16_t size` — shadow map size in pixels (valid range: 10–4096).
class ShadowMapSizeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SHADOW_MAP_SIZE"; }
};

/// @brief Shadow samples chunk (0x1430) — global number of shadow-map samples.
///
/// @details Optional (Release 1); appears at most once per MDATA. Total chunk size is 8 bytes.
///
/// @par Binary layout
/// - `int16_t samples` — number of shadow samples.
class ShadowSamplesChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SHADOW_SAMPLES"; }
};

/// @brief Shadow range chunk (0x1440) — global shadow influence range.
///
/// @details Optional (Release 1); appears at most once per MDATA. Total chunk size is 8 bytes.
///
/// @par Binary layout
/// - `int16_t range` — shadow range value.
class ShadowRangeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SHADOW_RANGE"; }
};

/// @brief Shadow filter chunk (0x1450) — global shadow-edge softness filter.
///
/// @details Optional (Release 2/3); appears at most once per MDATA. Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float filter` — filter width (range: 1.0–10.0).
class ShadowFilterChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SHADOW_FILTER"; }
};

/// @brief Ray bias chunk (0x1460) — global bias for ray-traced shadows.
///
/// @details Optional (Release 3); appears at most once per MDATA. Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float bias` — ray-traced shadow bias.
class RayBiasChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "RAY_BIAS"; }
};

/// @brief Construction plane origin chunk (0x1500) — 3D construction plane origin.
///
/// @details Optional (Release 2/3); appears at most once per MDATA. Total chunk size is 18 bytes.
///
/// @par Binary layout
/// - `float x, float y, float z` — origin of the construction plane.
class OConstsChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "O_CONSTS"; }
};

/// @}

// ============================================================================
/// @name Background environment chunks (children of MDATA)
/// @{
// ============================================================================

/// @brief Background bitmap chunk (0x1100) — sets a bitmap as the scene background.
///
/// @details Optional; appears at most once per MDATA. Total chunk size varies.
///
/// @par Binary layout
/// - `string filename` — NUL-terminated, up to 12 characters + NUL.
class BitMapChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "BIT_MAP"; }
};

/// @brief Use-background-bitmap flag chunk (0x1101) — activates the BIT_MAP background.
///
/// @details Optional; appears at most once per MDATA. No binary data (6-byte header only).
/// Must not appear unless BIT_MAP has also been defined.
class UseBitMapChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_BIT_MAP"; }
};

/// @brief Solid background color container chunk (0x1200) — defines a flat-color background.
///
/// @details Optional; appears at most once per MDATA.
/// Contains child color chunks; no direct binary payload.
///
/// @par Expected children
/// - COLOR_F (0x0010) — floating-point RGB (R1/R2).
/// - LIN_COLOR_F (0x0013) — linearised RGB (R3, preferred).
class SolidBgndChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SOLID_BGND"; }
};

/// @brief Use-solid-background flag chunk (0x1201) — activates the SOLID_BGND background.
///
/// @details Optional; appears at most once per MDATA. No binary data (6-byte header only).
/// Must not appear unless SOLID_BGND has also been defined.
class UseSolidBgndChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_SOLID_BGND"; }
};

/// @brief Vertical gradient background container chunk (0x1300) — three-band gradient background.
///
/// @details Optional; appears at most once per MDATA.
///
/// @par Binary layout (before children)
/// - `float midpoint` — relative vertical position of the mid-color band (0.0–1.0).
///
/// @par Expected children
/// Three top/mid/bottom COLOR_F (R1/R2) and three LIN_COLOR_F (R3) child chunks.
class VGradientChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "V_GRADIENT"; }
};

/// @brief Use-vertical-gradient flag chunk (0x1301) — activates the V_GRADIENT background.
///
/// @details Optional; appears at most once per MDATA. No binary data (6-byte header only).
/// Must not appear unless V_GRADIENT has also been defined.
class UseVGradientChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_V_GRADIENT"; }
};

/// @}

// ============================================================================
/// @name Atmosphere effect chunks (children of MDATA)
/// @{
// ============================================================================

/// @brief Fog effect container chunk (0x2200) — linear depth-cueing fog effect.
///
/// @details Optional; appears at most once per MDATA.
///
/// @par Binary layout (before children)
/// - `float nearDist`    — fog near distance (fog is 0% here).
/// - `float nearDensity` — fog density at the near plane (0.0–100.0).
/// - `float farDist`     — fog far distance (fog is `farDensity`% here).
/// - `float farDensity`  — fog density at the far plane (0.0–100.0).
///
/// @par Expected children
/// - COLOR_F (0x0010) — fog color.
/// - FOG_BGND (0x2210) — optional flag; applies fog to the background too.
class FogChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "FOG"; }
};

/// @brief Fog-affects-background flag chunk (0x2210).
///
/// @details Optional; child of FOG (0x2200). No binary data (6-byte header only).
/// When present, fog is also applied to the background image/color.
class FogBgndChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "FOG_BGND"; }
};

/// @brief Use-fog flag chunk (0x2201) — activates the FOG effect.
///
/// @details Optional; appears at most once per MDATA. No binary data (6-byte header only).
class UseFogChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_FOG"; }
};

/// @brief Layered fog container chunk (0x2302) — altitude-based volumetric fog (Release 3).
///
/// @details Optional (Release 3); appears at most once per MDATA.
///
/// @par Binary layout (before children)
/// - `float lowerZ`    — world-space lower Z boundary of the fog layer.
/// - `float upperZ`    — world-space upper Z boundary of the fog layer.
/// - `float density`   — fog density (0.0–100.0).
/// - `uint32_t flags`  — options:
///   - Bit 0 (0x1) — fog falls off (density reduces with altitude).
///   - Bit 1 (0x2) — use background color for fog.
///
/// @par Expected children
/// - COLOR_F (0x0010) — fog color (only used if flag bit 1 is clear).
class LayerFogChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "LAYER_FOG"; }
};

/// @brief Use-layer-fog flag chunk (0x2303) — activates the LAYER_FOG effect (Release 3).
///
/// @details Optional (Release 3); appears at most once per MDATA. No binary data (6-byte header only).
class UseLayerFogChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_LAYER_FOG"; }
};

/// @brief Distance cue container chunk (0x2300) — non-linear distance-based dimming effect.
///
/// @details Optional; appears at most once per MDATA.
///
/// @par Binary layout (no children, all data inline)
/// - `float nearDist`    — distance at which dimming begins.
/// - `float nearDimming` — dimming amount at near distance (0.0 = no dim).
/// - `float farDist`     — distance at which full dimming is reached.
/// - `float farDimming`  — dimming amount at far distance (0.0–100.0).
///
/// @par Optional child
/// - DCUE_BGND (0x2310) — applies distance cue to the background too.
class DistanceCueChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DISTANCE_CUE"; }
};

/// @brief Distance-cue-affects-background flag chunk (0x2310).
///
/// @details Optional; child of DISTANCE_CUE (0x2300). No binary data (6-byte header only).
class DcueBgndChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DCUE_BGND"; }
};

/// @brief Use-distance-cue flag chunk (0x2301) — activates the DISTANCE_CUE effect.
///
/// @details Optional; appears at most once per MDATA. No binary data (6-byte header only).
class UseDistanceCueChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "USE_DISTANCE_CUE"; }
};

/// @}

// ============================================================================
/// @name Viewport layout chunks (children of MDATA, Release 3)
/// @{
// ============================================================================

/// @brief Viewport layout container chunk (0x7001) — describes the multi-viewport arrangement.
///
/// @details Optional; appears at most once per MDATA. Contains global layout parameters
/// and one or more viewport definition children.
///
/// @par Binary layout (before children)
/// - `int16_t flags`  — layout flags.
/// - `int16_t style`  — layout style index (0–11 matching 3D Studio layouts).
/// - `int16_t active` — index of the active viewport.
/// - `int16_t[3] _`   — internal padding/unused fields.
///
/// @par Expected children
/// - VIEWPORT_SIZE (0x7020) — required, overall size and position.
/// - VIEWPORT_DATA_3 (0x7012) — one per viewport (R3).
/// - VIEWPORT_DATA (0x7011) — one per viewport (R1/R2).
class ViewportLayoutChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEWPORT_LAYOUT"; }
};

/// @brief Viewport size chunk (0x7020) — pixel dimensions of the viewport area.
///
/// @details Appears exactly once per VIEWPORT_LAYOUT. Total chunk size is 14 bytes.
///
/// @par Binary layout
/// - `uint16_t x`      — left edge in screen pixels.
/// - `uint16_t y`      — top edge in screen pixels.
/// - `uint16_t width`  — viewport width in pixels.
/// - `uint16_t height` — viewport height in pixels.
class ViewportSizeChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEWPORT_SIZE"; }
};

/// @brief Viewport data chunk — Release 1/2 viewport descriptor (0x7012).
///
/// @details Optional; appears once per sub-viewport inside VIEWPORT_LAYOUT.
/// Total chunk size is 61 bytes; binary format is the same as VIEWPORT_DATA_3
/// except for field interpretation differences in early releases.
class ViewportDataChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEWPORT_DATA"; }
};

/// @brief Viewport data chunk — Release 3 viewport descriptor (0x7011).
///
/// @details Optional; appears once per sub-viewport inside VIEWPORT_LAYOUT.
/// Total chunk size is 61 bytes.
///
/// @par Binary layout
/// - `int16_t flags` — viewport flags.
/// - `int16_t[5] _`  — internal axis/view data.
/// - `float[3] zoom` — zoom/pan/focus values.
/// - `float[3] worldCenter` — world-space center coordinates.
/// - `float worldWidth`     — visible world width at current zoom.
/// - `int16_t viewType`     — view type (0=none, 1=top, 2=bottom, 3=left, 4=right, 5=front, 6=back, 7=user, 18=camera, 65=spotlight).
/// - `int16_t cameraOrLight` — index of camera or light for view types 18/65.
/// - `string  name`          — NUL-terminated name of camera/light (up to 11 characters).
class ViewportData3Chunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEWPORT_DATA_3"; }
};

/// @}

// ============================================================================
/// @name Default view chunks (children of MDATA, Release 1/2)
/// @{
// ============================================================================

/// @brief Default view container chunk (0x3000) — stores the startup viewport state.
///
/// @details Optional (Release 1/2); appears at most once per MDATA.
/// Contains exactly one of the view sub-chunks below as its only child.
class DefaultViewChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "DEFAULT_VIEW"; }
};

/// @brief Top orthographic view data chunk (0x3010).
///
/// @details Optional; child of DEFAULT_VIEW. Total chunk size is 22 bytes.
///
/// @par Binary layout
/// - `float[3] target` — 3D target/focus point.
/// - `float width`     — visible width in world units.
class ViewTopChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_TOP"; }
};

/// @brief Bottom orthographic view data chunk (0x3020).
///
/// @details Optional; child of DEFAULT_VIEW. Same binary layout as VIEW_TOP.
class ViewBottomChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_BOTTOM"; }
};

/// @brief Left orthographic view data chunk (0x3030).
///
/// @details Optional; child of DEFAULT_VIEW. Same binary layout as VIEW_TOP.
class ViewLeftChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_LEFT"; }
};

/// @brief Right orthographic view data chunk (0x3040).
///
/// @details Optional; child of DEFAULT_VIEW. Same binary layout as VIEW_TOP.
class ViewRightChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_RIGHT"; }
};

/// @brief Front orthographic view data chunk (0x3050).
///
/// @details Optional; child of DEFAULT_VIEW. Same binary layout as VIEW_TOP.
class ViewFrontChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_FRONT"; }
};

/// @brief Back orthographic view data chunk (0x3060).
///
/// @details Optional; child of DEFAULT_VIEW. Same binary layout as VIEW_TOP.
class ViewBackChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_BACK"; }
};

/// @brief User perspective view data chunk (0x3070) — free-angle user view.
///
/// @details Optional; child of DEFAULT_VIEW. Total chunk size is 38 bytes.
///
/// @par Binary layout
/// - `float[3] target`     — 3D target/focus point.
/// - `float width`         — visible width in world units.
/// - `float horizAngle`    — horizontal rotation angle (degrees).
/// - `float vertAngle`     — vertical rotation angle (degrees).
/// - `float bankAngle`     — bank/roll angle (degrees).
class ViewUserChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_USER"; }
};

/// @brief Camera view data chunk (0x3080) — default view through a named camera.
///
/// @details Optional; child of DEFAULT_VIEW. Total chunk size varies.
///
/// @par Binary layout
/// - `string name` — NUL-terminated camera object name (1–10 characters + NUL).
class ViewCameraChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "VIEW_CAMERA"; }
};

/// @}

} // namespace Debugger3DS::Parser::Chunks