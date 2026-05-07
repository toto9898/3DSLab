#pragma once

#include "Chunk.h"
#include <string>
#include <memory>

namespace Debugger3DS::Scene { struct Material; }

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::Material;
// Importer is in parent namespace Debugger3DS::Parser — accessible as Importer below

/// @brief Base for chunks that need the currently-active `Material` from the importer.
///
/// Provides a default `ReadData()` that resolves the material pointer.  Container
/// sub-chunks (ambient, diffuse, etc.) that carry no data of their own can
/// inherit from this class without overriding `ReadData()`.
class MaterialContainerChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;

protected:
    /// @brief Resolve and cache the current material from @p importer.
    /// @return @c true if a current material is available.
    bool GetCurrentMaterial(Importer& importer);

    std::shared_ptr<Material> targetMaterial_; ///< Resolved current material.
};

// ============================================================================
/// @name Material name chunk (0xAFFF)
/// @{
// ============================================================================

/// @brief Material name chunk (0xA000) — creates a new `Material` and makes it current.
///
/// @details Required; appears exactly once per MAT_ENTRY (0xAFFF). Names must
/// be unique across all material entries in the file. All subsequent material
/// sub-chunks (ambient, diffuse, texmap, etc.) apply to this material.
///
/// @par Binary layout
/// - `string name` — NUL-terminated, 1–16 characters + NUL terminator.
class MatNameChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "MAT_NAME"; }

    /// @brief Return the parsed material name.
    const std::string& GetMaterialName() const;

private:
    std::string name_;                              ///< Parsed material name.
    std::shared_ptr<Material> targetMaterial_ = nullptr; ///< Newly-created material.
};

/// @}

// ============================================================================
/// @name Material color chunks (0xA010, 0xA020, 0xA030)
/// @{
// ============================================================================

/// @brief Ambient color container chunk (0xA010) — sets the material's ambient light color.
///
/// @details Pure container; the actual RGB values come from child color sub-chunks.
/// Appears at most once per MAT_ENTRY.
///
/// @par Expected children
/// - COLOR_24 (0x0011) — 24-bit color (R1 and R2).
/// - LIN_COLOR_24 (0x0012) — linearised 24-bit color (R3, preferred).
class MatAmbientChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_AMBIENT"; }
};

/// @brief Diffuse color container chunk (0xA020) — sets the material's diffuse (base) color.
///
/// @details Pure container; the actual RGB values come from child color sub-chunks.
/// Appears at most once per MAT_ENTRY.
///
/// @par Expected children
/// - COLOR_24 (0x0011) — 24-bit color (R1 and R2).
/// - LIN_COLOR_24 (0x0012) — linearised 24-bit color (R3, preferred).
class MatDiffuseChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_DIFFUSE"; }
};

/// @brief Specular color container chunk (0xA030) — sets the material's specular highlight color.
///
/// @details Pure container; the actual RGB values come from child color sub-chunks.
/// Appears at most once per MAT_ENTRY.
///
/// @par Expected children
/// - COLOR_24 (0x0011) — 24-bit color (R1 and R2).
/// - LIN_COLOR_24 (0x0012) — linearised 24-bit color (R3, preferred).
class MatSpecularChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SPECULAR"; }
};

/// @}

// ============================================================================
/// @name Shininess chunks (0xA040, 0xA041)
/// @{
// ============================================================================

/// @brief Shininess container chunk (0xA040) — sets the material's shininess ratio.
///
/// @details Pure container; the percentage value comes from a child INT_PERCENTAGE chunk.
/// Shininess ranges from 0 (matte) to 100 (shiny). Appears at most once per MAT_ENTRY.
class MatShininessChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SHININESS"; }
};

/// @brief Shininess strength container chunk (0xA041) — sets the shininess strength (specular level).
///
/// @details Pure container; value from a child INT_PERCENTAGE chunk (0–100).
/// Appears at most once per MAT_ENTRY. Added in Release 3.
class MatShin2PctChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SHIN2PCT"; }
};

/// @}

// ============================================================================
/// @name Transparency chunks (0xA050, 0xA052, 0xA053)
/// @{
// ============================================================================

/// @brief Transparency container chunk (0xA050) — sets the material's transparency ratio.
///
/// @details Pure container; value from a child INT_PERCENTAGE chunk.
/// Range: 0 (fully opaque) to 100 (fully transparent). Appears at most once per MAT_ENTRY.
class MatTransparencyChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TRANSPARENCY"; }
};

/// @brief Transparency falloff container chunk (0xA052) — centre-to-edge transparency falloff.
///
/// @details Pure container; value from a child INT_PERCENTAGE chunk.
/// Range: −100 to +100. The magnitude controls falloff strength; the sign
/// controls direction: negative = inside falloff, positive = outside falloff.
/// Optional; found in files from Release 2 and 3.
class MatXpfallChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_XPFALL"; }
};

/// @brief Reflection blur container chunk (0xA053) — sets the reflection blurring ratio.
///
/// @details Pure container; value from a child INT_PERCENTAGE chunk (0–100).
/// Optional; found in files from Release 2 and 3.
class MatRefblurChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_REFBLUR"; }
};

/// @}

// ============================================================================
/// @name Material property chunks (0xA084, 0xA087, 0xA100)
/// @{
// ============================================================================

/// @brief Self-illumination container chunk (0xA084) — sets the self-illumination percentage.
///
/// @details Pure container; value from a child INT_PERCENTAGE chunk (0–100).
/// Optional; found in files from Release 3 (older files use a flag chunk).
class MatSelfIllumChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SELF_ILLUM"; }
};

/// @brief Wire-size chunk (0xA087) — wireframe rendering line width for the current material.
///
/// @details Optional; found in Release 3 files. Appears at most once per MAT_ENTRY.
///
/// @par Binary layout
/// - `float width` — line width. Units are screen pixels by default; switches to
///   world units if a MAT_WIREABS (0xA08E) flag chunk is also present in the same MAT_ENTRY.
class MatWireSizeChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "MAT_WIRE_SIZE"; }
};

/// @brief Shading method chunk (0xA100) — selects the rendering shading model.
///
/// @details Required; appears exactly once per MAT_ENTRY.
///
/// @par Binary layout
/// - `int16_t shading` — shading model identifier:
///   - 0 — Wireframe (Release 1 and 2 only).
///   - 1 — Flat shading.
///   - 2 — Gouraud shading.
///   - 3 — Phong shading.
///   - 4 — Metal shading.
class MatShadingChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "MAT_SHADING"; }
};

/// @}

// ============================================================================
/// @name Texture map chunks (0xA200, 0xA300)
/// @{
// ============================================================================

/// @brief Texture map container chunk (0xA200) — first/primary diffuse texture map definition.
///
/// @details Optional; appears at most once per MAT_ENTRY. Contains the map
/// filename and all associated parameters as child chunks.
///
/// @par Required children
/// - INT_PERCENTAGE (0x0030) — texture map blend strength (0–100).
/// - MAT_MAPNAME (0xA300) — bitmap filename.
///
/// @par Optional children (R3)
/// - MAT_MAP_TILING (0xA351) — tiling and control flags.
/// - MAT_MAT_TEXBLUR (0xA353) — texture blurring factor.
/// - MAT_MAP_USCALE (0xA354) — U coordinate scale.
/// - MAT_MAP_VSCALE (0xA356) — V coordinate scale.
/// - MAT_MAP_UOFFSET (0xA358) — U coordinate offset.
/// - MAT_MAP_VOFFSET (0xA35A) — V coordinate offset.
/// - MAT_MAP_ANG (0xA35C) — map rotation angle.
/// - MAT_MAP_COL1 (0xA360) — first tinting color.
/// - MAT_MAP_COL2 (0xA362) — second tinting color.
/// - MAT_MAP_RCOL (0xA364) — red component tint color.
/// - MAT_MAP_GCOL (0xA366) — green component tint color.
/// - MAT_MAP_BCOL (0xA368) — blue component tint color.
class MatTexmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TEXMAP"; }
};

/// @brief Texture map filename chunk (0xA300) — stores the bitmap filename for the enclosing map.
///
/// @details Appears exactly once per map container chunk (MAT_TEXMAP, MAT_BUMPMAP, etc.).
/// The filename extension is required for 3D Studio to identify the bitmap format.
/// The file should reside in a configured map-path directory.
///
/// @par Binary layout
/// - `string filename` — NUL-terminated, up to 12 characters + NUL terminator.
class MatMapNameChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "MAT_MAPNAME"; }
};

/// @}

// ============================================================================
/// @name Material property flag chunks
/// @{
// ============================================================================

/// @brief Two-sided flag chunk (0xA081) — makes the material render on both faces.
///
/// @details Optional; appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatTwoSideChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TWO_SIDE"; }
};

/// @brief Decal mapping flag chunk (0xA082) — enables decal (non-tiling) texture mapping.
///
/// @details Optional (Release 1/2); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatDecalChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_DECAL"; }
};

/// @brief Additive transparency flag chunk (0xA083) — enables additive (glow) transparency.
///
/// @details Optional; appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatAdditiveChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_ADDITIVE"; }
};

/// @brief Wireframe rendering flag chunk (0xA085) — renders the material as a wireframe.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatWireChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_WIRE"; }
};

/// @brief Face-mapped textures flag chunk (0xA088) — applies the texture to each face individually.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatFacemapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_FACEMAP"; }
};

/// @brief Phong soften flag chunk (0xA08C) — softens Phong highlights on this material.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
class MatPhongsoftChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_PHONGSOFT"; }
};

/// @brief Wire size in world-units flag chunk (0xA08E) — switches wireframe width to world units.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
/// When present, the wire width in MAT_WIRE_SIZE (0xA087) is interpreted as world units
/// rather than screen pixels.
class MatWireAbsChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_WIREABS"; }
};

/// @brief Use-transparency-falloff flag chunk (0xA240) — activates falloff defined by MAT_XPFALL.
///
/// @details Optional (Release 2/3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
/// Must not appear unless MAT_XPFALL (0xA052) has been defined.
class MatUseXpfallChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_USE_XPFALL"; }
};

/// @brief Use-reflection-blur flag chunk (0xA250) — activates blur defined by MAT_REFBLUR.
///
/// @details Optional (Release 2/3); appears at most once per MAT_ENTRY. No binary data (6-byte header only).
/// Must not appear unless MAT_REFBLUR (0xA053) has been defined.
class MatUseRefblurChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_USE_REFBLUR"; }
};

/// @}

// ============================================================================
/// @name Additional texture map container chunks (0xA204–0xA33D)
/// @{
// ============================================================================

/// @brief Specular map container chunk (0xA204) — specular highlight texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and all MAT_MAP_* property children.
/// Also supports MAT_MAP_COL1/COL2/RCOL/GCOL/BCOL tinting children.
class MatSpecmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SPECMAP"; }
};

/// @brief Opacity map container chunk (0xA210) — opacity/transparency texture map.
///
/// @details Optional; appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and all MAT_MAP_* property children.
class MatOpacmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_OPACMAP"; }
};

/// @brief Reflection map container chunk (0xA220) — environment/reflection texture map.
///
/// @details Optional; appears at most once per MAT_ENTRY.
/// Contains INT_PERCENTAGE strength and MAT_MAPNAME filename.
/// An empty filename string indicates automatic reflection maps are in use
/// (defined by MAT_ACUBIC).
class MatReflmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_REFLMAP"; }
};

/// @brief Bump map container chunk (0xA230) — surface bump/normal texture map.
///
/// @details Optional; appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and all MAT_MAP_* property children.
class MatBumpmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_BUMPMAP"; }
};

/// @brief Second texture map container chunk (0xA33A) — secondary diffuse texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200), including
/// INT_PERCENTAGE, MAT_MAPNAME, all MAT_MAP_* property children, and
/// MAT_MAP_COL1/COL2/RCOL/GCOL/BCOL tinting children.
class MatTex2mapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TEX2MAP"; }
};

/// @brief Shininess map container chunk (0xA33C) — shininess variation texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and all MAT_MAP_* property children.
class MatShinmapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SHINMAP"; }
};

/// @brief Self-illumination map container chunk (0xA33D) — self-illumination texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and all MAT_MAP_* property children.
class MatSelfimapChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SELFIMAP"; }
};

/// @}

// ============================================================================
/// @name Texture mask container chunks (0xA33E–0xA34C)
/// @{
// ============================================================================

/// @brief First texture mask container chunk (0xA33E) — mask for the primary texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMAP (0xA200):
/// INT_PERCENTAGE strength, MAT_MAPNAME filename, and MAT_MAP_* property children.
class MatTexmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TEXMASK"; }
};

/// @brief Second texture mask container chunk (0xA340) — mask for the secondary texture map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatTex2maskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_TEX2MASK"; }
};

/// @brief Opacity mask container chunk (0xA342) — mask for the opacity map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatOpacmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_OPACMASK"; }
};

/// @brief Bump mask container chunk (0xA344) — mask for the bump map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatBumpmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_BUMPMASK"; }
};

/// @brief Shininess mask container chunk (0xA346) — mask for the shininess map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatShinmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SHINMASK"; }
};

/// @brief Specular mask container chunk (0xA348) — mask for the specular map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatSpecmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SPECMASK"; }
};

/// @brief Reflection mask container chunk (0xA34A) — mask for the reflection map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatReflmaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_REFLMASK"; }
};

/// @brief Self-illumination mask container chunk (0xA34C) — mask for the self-illumination map.
///
/// @details Optional (Release 3); appears at most once per MAT_ENTRY.
/// Shares the same child chunk structure as MAT_TEXMASK.
class MatSelfimaskChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SELFIMASK"; }
};

/// @}

// ============================================================================
/// @name Automatic cubic reflection chunk (0xA310)
/// @{
// ============================================================================

/// @brief Automatic cubic reflection map chunk (0xA310) — parameters for auto-generated environment maps.
///
/// @details Optional (Release 2/3); appears only when automatic reflection maps
/// are defined (MAT_REFLMAP with an empty filename). Total chunk size is 18 bytes.
///
/// @par Binary layout
/// - `uint8_t  shadeLevel`  — shade level; not used, should be 0.
/// - `uint8_t  antiAlias`   — anti-alias level: 0=None, 1=Low, 2=Medium, 3=High (R2 only).
/// - `int16_t  flags`       — reflection options:
///   - Bit 0 (0x1) — enable auto cubic map.
///   - Bit 2 (0x4) — render first frame only.
///   - Bit 3 (0x8) — flat mirroring.
/// - `int32_t  mapSize`     — size of the generated reflection map in pixels.
/// - `int32_t  nthFrame`    — frame interval for generating new reflection maps.
class MatAcubicChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_ACUBIC"; }
};

/// @}

// ============================================================================
/// @name Map parameter chunks (0xA351–0xA368)
/// @{
// ============================================================================

/// @brief Map tiling control flags chunk (0xA351) — controls tiling, mirroring, and sampling options.
///
/// @details Required (Release 3); appears at most once per map container chunk.
/// Total chunk size is 8 bytes.
///
/// @par Binary layout
/// - `uint16_t flags` — control bits:
///   - Bit 0 (0x001) — activate decaling.
///   - Bit 1 (0x002) — activate mirroring.
///   - Bit 3 (0x008) — activate negation.
///   - Bit 4 (0x010) — deactivate tiling.
///   - Bit 5 (0x020) — activate summed-area sampling.
///   - Bit 6 (0x040) — activate alpha sourcing (requires bit 7 or 9 for texmap/tex2map/specmap).
///   - Bit 7 (0x080) — activate RGB tinting (texmap/tex2map/specmap only).
///   - Bit 8 (0x100) — ignore alpha channel.
///   - Bit 9 (0x200) — activate RGB tinting (alternate; texmap/tex2map/specmap only).
class MatMapTilingChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_TILING"; }
};

/// @brief Map texture blur factor chunk (0xA353) — blurring applied to the map.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float blur` — blur factor (0.0 = no blur, 1.0 = maximum blur).
class MatMapTexblurChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_TEXBLUR"; }
};

/// @brief Map U-coordinate scale chunk (0xA354) — horizontal tiling scale factor.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float scale` — U scale value (any non-zero positive or negative float).
class MatMapUscaleChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_USCALE"; }
};

/// @brief Map V-coordinate scale chunk (0xA356) — vertical tiling scale factor.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float scale` — V scale value (any non-zero positive or negative float).
class MatMapVscaleChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_VSCALE"; }
};

/// @brief Map U-coordinate offset chunk (0xA358) — horizontal translation of the map.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float offset` — U offset value.
class MatMapUoffsetChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_UOFFSET"; }
};

/// @brief Map V-coordinate offset chunk (0xA35A) — vertical translation of the map.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float offset` — V offset value.
class MatMapVoffsetChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_VOFFSET"; }
};

/// @brief Map rotation angle chunk (0xA35C) — rotation of the texture map about its center.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Total chunk size is 10 bytes.
///
/// @par Binary layout
/// - `float angle` — rotation angle in degrees (range: −360.0 to +360.0).
class MatMapAngChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_ANG"; }
};

/// @brief First map tinting color chunk (0xA360) — first RGB color used for map tinting.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Only valid for MAT_TEXMAP, MAT_TEX2MAP, and MAT_SPECMAP. Total chunk size is 9 bytes.
///
/// @par Binary layout
/// - `uint8_t r, uint8_t g, uint8_t b` — RGB values (0–255 each).
class MatMapCol1Chunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_COL1"; }
};

/// @brief Second map tinting color chunk (0xA362) — second RGB color used for map tinting.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Only valid for MAT_TEXMAP, MAT_TEX2MAP, and MAT_SPECMAP. Total chunk size is 9 bytes.
///
/// @par Binary layout
/// - `uint8_t r, uint8_t g, uint8_t b` — RGB values (0–255 each).
class MatMapCol2Chunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_COL2"; }
};

/// @brief Red-channel tinting color chunk (0xA364) — color applied to the map's red channel.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Only valid for MAT_TEXMAP, MAT_TEX2MAP, and MAT_SPECMAP. Total chunk size is 9 bytes.
///
/// @par Binary layout
/// - `uint8_t r, uint8_t g, uint8_t b` — RGB values (0–255 each).
class MatMapRcolChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_RCOL"; }
};

/// @brief Green-channel tinting color chunk (0xA366) — color applied to the map's green channel.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Only valid for MAT_TEXMAP, MAT_TEX2MAP, and MAT_SPECMAP. Total chunk size is 9 bytes.
///
/// @par Binary layout
/// - `uint8_t r, uint8_t g, uint8_t b` — RGB values (0–255 each).
class MatMapGcolChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_GCOL"; }
};

/// @brief Blue-channel tinting color chunk (0xA368) — color applied to the map's blue channel.
///
/// @details Optional (Release 3); appears at most once per map container chunk.
/// Only valid for MAT_TEXMAP, MAT_TEX2MAP, and MAT_SPECMAP. Total chunk size is 9 bytes.
///
/// @par Binary layout
/// - `uint8_t r, uint8_t g, uint8_t b` — RGB values (0–255 each).
class MatMapBcolChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_MAP_BCOL"; }
};

/// @}

// ============================================================================
/// @name SXP procedural data chunks (0xA320–0xA338)
/// @{
// ============================================================================

/// @brief SXP primary texture procedural data chunk (0xA320).
///
/// @details Optional (Release 2/3); present only when MAT_MAPNAME for the primary texture
/// map refers to a solid procedure (.sxp extension). Content is procedure-specific.
class MatSxpTextDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_TEXT_DATA"; }
};

/// @brief SXP second texture procedural data chunk (0xA321).
///
/// @details Optional (Release 3); present only when MAT_MAPNAME for the secondary texture
/// map refers to a solid procedure (.sxp extension). Content is procedure-specific.
class MatSxpText2DataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_TEXT2_DATA"; }
};

/// @brief SXP opacity procedural data chunk (0xA322).
///
/// @details Optional; present only when MAT_MAPNAME for the opacity map refers
/// to a solid procedure (.sxp extension). Content is procedure-specific.
class MatSxpOpacDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_OPAC_DATA"; }
};

/// @brief SXP texture-mask procedural data chunk (0xA324).
///
/// @details Optional (Release 3); present only when MAT_MAPNAME for the primary texture mask
/// refers to a solid procedure (.sxp extension). Content is procedure-specific.
/// Note: chunk ID 0xA323 is not defined; this entry follows 0xA322 directly.
class MatSxpTextMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_TEXT_MASKDATA"; }
};

/// @brief SXP specular procedural data chunk (0xA325).
///
/// @details Optional (Release 3); present only when MAT_MAPNAME for the specular map
/// refers to a solid procedure (.sxp extension). Content is procedure-specific.
class MatSxpSpecDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_SPEC_DATA"; }
};

/// @brief SXP second-texture-mask procedural data chunk (0xA326).
///
/// @details Optional (Release 3). Content is procedure-specific.
class MatSxpText2MaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_TEXT2MASK_DATA"; }
};

/// @brief SXP self-illumination procedural data chunk (0xA327).
///
/// @details Optional (Release 3); present only when MAT_MAPNAME for the self-illumination map
/// refers to a solid procedure (.sxp extension). Content is procedure-specific.
class MatSxpSelfiDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_SELFI_DATA"; }
};

/// @brief SXP opacity-mask procedural data chunk (0xA328).
///
/// @details Optional (Release 2/3). Content is procedure-specific.
/// Note: chunk ID 0xA329 is not defined; the next entry is at 0xA32A.
class MatSxpOpacMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_OPACMASK_DATA"; }
};

/// @brief SXP bump-mask procedural data chunk (0xA32A).
///
/// @details Optional (Release 3). Content is procedure-specific.
/// Note: chunk IDs 0xA329 and 0xA32B are not defined.
class MatSxpBumpMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_BUMPMASK_DATA"; }
};

/// @brief SXP specular-mask procedural data chunk (0xA32C).
///
/// @details Optional (Release 3). Content is procedure-specific.
/// Note: chunk ID 0xA32D is not defined.
class MatSxpSpecMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_SPECMASK_DATA"; }
};

/// @brief SXP shininess-mask procedural data chunk (0xA32E).
///
/// @details Optional (Release 3). Content is procedure-specific.
/// Note: chunk ID 0xA32F is not defined.
class MatSxpShinMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_SHINMASK_DATA"; }
};

/// @brief SXP self-illumination-mask procedural data chunk (0xA330).
///
/// @details Optional (Release 2/3). Content is procedure-specific.
/// Note: chunk ID 0xA331 is not defined.
class MatSxpSelfiMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_SELFIMASK_DATA"; }
};

/// @brief SXP reflection-mask procedural data chunk (0xA332).
///
/// @details Optional (Release 3). Content is procedure-specific.
class MatSxpReflMaskDataChunk : public MaterialContainerChunk {
public:
    using MaterialContainerChunk::MaterialContainerChunk;
    std::string GetTypeName() const override { return "MAT_SXP_REFL_MASKDATA"; }
};

/// @}

} // namespace Debugger3DS::Parser::Chunks
