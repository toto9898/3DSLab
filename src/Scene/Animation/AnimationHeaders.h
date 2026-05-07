#pragma once

#include <cstdint>
#include <string>
#include <istream>
#include <sstream>

namespace Debugger3DS::Scene {

/// @brief Binary header read at the start of every animation track in the KFDATA section.
///
/// Each animated property (position, rotation, scale, etc.) is preceded by this
/// 10-byte structure that describes how many keyframes follow and optional flags
/// controlling looping and axis locking.
struct TrackHeader {
    uint16_t flags    = 0; ///< Bitmask of track flags (see FLAG_* constants).
    uint32_t unknown1 = 0; ///< Reserved; always zero in conforming files.
    uint32_t unknown2 = 0; ///< Reserved; always zero in conforming files.
    uint32_t keyCount = 0; ///< Number of keyframes that follow this header.

    /// @brief Read the header fields from a binary stream.
    /// @param stream Input stream positioned at the first byte of the header.
    /// @return @c true on success.
    bool Read(std::istream& stream);

    /// @brief Build a human-readable summary for logging.
    std::string GetInfo() const;

    /// @brief Return @c true if the header was read successfully and @c keyCount is non-zero.
    bool IsValid() const;

    /// @name Track flag constants
    /// @{
    static constexpr uint16_t FLAG_LOOP     = 0x0001; ///< Track loops at the end of the segment.
    static constexpr uint16_t FLAG_LOCK_X   = 0x0002; ///< X axis is locked (no animation).
    static constexpr uint16_t FLAG_LOCK_Y   = 0x0004; ///< Y axis is locked.
    static constexpr uint16_t FLAG_LOCK_Z   = 0x0008; ///< Z axis is locked.
    static constexpr uint16_t FLAG_UNLINK_X = 0x0010; ///< X axis is unlinked from parent.
    static constexpr uint16_t FLAG_UNLINK_Y = 0x0020; ///< Y axis is unlinked.
    static constexpr uint16_t FLAG_UNLINK_Z = 0x0040; ///< Z axis is unlinked.
    /// @}

    /// @name Flag query helpers
    /// @{
    bool IsLooped()    const { return (flags & FLAG_LOOP)     != 0; } ///< @return Whether the track loops.
    bool IsXLocked()   const { return (flags & FLAG_LOCK_X)   != 0; } ///< @return Whether the X axis is locked.
    bool IsYLocked()   const { return (flags & FLAG_LOCK_Y)   != 0; } ///< @return Whether the Y axis is locked.
    bool IsZLocked()   const { return (flags & FLAG_LOCK_Z)   != 0; } ///< @return Whether the Z axis is locked.
    bool IsXUnlinked() const { return (flags & FLAG_UNLINK_X) != 0; } ///< @return Whether the X axis is unlinked.
    bool IsYUnlinked() const { return (flags & FLAG_UNLINK_Y) != 0; } ///< @return Whether the Y axis is unlinked.
    bool IsZUnlinked() const { return (flags & FLAG_UNLINK_Z) != 0; } ///< @return Whether the Z axis is unlinked.
    /// @}
};

/// @brief Per-keyframe header read before each keyframe value in an animation track.
///
/// After the frame number and flags, optional TCB (Tension/Continuity/Bias) spline
/// parameters and ease-in/out values may follow; their presence is indicated by bits
/// in @c flags.
struct KeyHeader {
    uint32_t frame       = 0;    ///< Frame index of this keyframe.
    uint16_t flags       = 0;    ///< Bitmask controlling which spline fields are present.

    float tension    = 0.0f; ///< TCB tension  (present if FLAG_USE_TENSION is set).
    float continuity = 0.0f; ///< TCB continuity (present if FLAG_USE_CONTINUITY is set).
    float bias       = 0.0f; ///< TCB bias      (present if FLAG_USE_BIAS is set).
    float easeTo     = 0.0f; ///< Ease-to value  (present if FLAG_USE_EASE_TO is set).
    float easeFrom   = 0.0f; ///< Ease-from value (present if FLAG_USE_EASE_FROM is set).

    /// @brief Read frame index, flags, and any optional spline data from @p stream.
    /// @param stream Input stream positioned at the first byte of this key header.
    /// @return @c true on success.
    bool Read(std::istream& stream);

    /// @brief Build a human-readable summary for logging.
    std::string GetInfo() const;

    /// @brief Return @c true if the key header was read without stream errors.
    bool IsValid() const;

    /// @name Keyframe flag constants
    /// @{
    static constexpr uint16_t FLAG_USE_TENSION    = 0x0001; ///< Tension field is present.
    static constexpr uint16_t FLAG_USE_CONTINUITY = 0x0002; ///< Continuity field is present.
    static constexpr uint16_t FLAG_USE_BIAS       = 0x0004; ///< Bias field is present.
    static constexpr uint16_t FLAG_USE_EASE_TO    = 0x0008; ///< Ease-to field is present.
    static constexpr uint16_t FLAG_USE_EASE_FROM  = 0x0010; ///< Ease-from field is present.
    /// @}

    /// @name Flag query helpers
    /// @{
    bool UsesTension()    const { return (flags & FLAG_USE_TENSION)    != 0; } ///< @return Whether tension data is present.
    bool UsesContinuity() const { return (flags & FLAG_USE_CONTINUITY) != 0; } ///< @return Whether continuity data is present.
    bool UsesBias()       const { return (flags & FLAG_USE_BIAS)       != 0; } ///< @return Whether bias data is present.
    bool UsesEaseTo()     const { return (flags & FLAG_USE_EASE_TO)    != 0; } ///< @return Whether ease-to data is present.
    bool UsesEaseFrom()   const { return (flags & FLAG_USE_EASE_FROM)  != 0; } ///< @return Whether ease-from data is present.
    /// @}

    /// @brief Return the total byte size of this key header (including optional fields).
    size_t GetSize() const;

private:
    /// @brief Read optional TCB/ease spline fields based on @c flags.
    bool ReadSplineData(std::istream& stream);
};

} // namespace Debugger3DS::Scene