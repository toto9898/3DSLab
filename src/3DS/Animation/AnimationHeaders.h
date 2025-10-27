#pragma once

#include <cstdint>
#include <string>
#include <istream>
#include <sstream>

namespace Debugger3DS {

    // Track header structure for animation tracks
    struct TrackHeader {
        uint16_t flags = 0;        // Track flags (loop, etc.)
        uint32_t unknown1 = 0;     // Unknown field 1
        uint32_t unknown2 = 0;     // Unknown field 2
        uint32_t keyCount = 0;     // Number of keyframes
        
        // Read track header from stream
        bool Read(std::istream& stream);
        
        // Get info string for debugging
        std::string GetInfo() const;
        
        // Check if track is valid
        bool IsValid() const;
        
        // Track flag constants
        static constexpr uint16_t FLAG_LOOP = 0x0001;
        static constexpr uint16_t FLAG_LOCK_X = 0x0002;
        static constexpr uint16_t FLAG_LOCK_Y = 0x0004;
        static constexpr uint16_t FLAG_LOCK_Z = 0x0008;
        static constexpr uint16_t FLAG_UNLINK_X = 0x0010;
        static constexpr uint16_t FLAG_UNLINK_Y = 0x0020;
        static constexpr uint16_t FLAG_UNLINK_Z = 0x0040;
        
        // Flag check helpers
        bool IsLooped() const { return (flags & FLAG_LOOP) != 0; }
        bool IsXLocked() const { return (flags & FLAG_LOCK_X) != 0; }
        bool IsYLocked() const { return (flags & FLAG_LOCK_Y) != 0; }
        bool IsZLocked() const { return (flags & FLAG_LOCK_Z) != 0; }
        bool IsXUnlinked() const { return (flags & FLAG_UNLINK_X) != 0; }
        bool IsYUnlinked() const { return (flags & FLAG_UNLINK_Y) != 0; }
        bool IsZUnlinked() const { return (flags & FLAG_UNLINK_Z) != 0; }
    };
    
    // Key header structure for individual keyframes
    struct KeyHeader {
        uint32_t frame = 0;        // Frame number
        uint16_t flags = 0;        // Keyframe flags
        
        // Spline control data (optional, depends on flags)
        float tension = 0.0f;      // Spline tension
        float continuity = 0.0f;   // Spline continuity  
        float bias = 0.0f;         // Spline bias
        float easeTo = 0.0f;       // Ease to value
        float easeFrom = 0.0f;     // Ease from value
        
        // Read key header from stream
        bool Read(std::istream& stream);
        
        // Get info string for debugging
        std::string GetInfo() const;
        
        // Check if key is valid
        bool IsValid() const;
        
        // Keyframe flag constants
        static constexpr uint16_t FLAG_USE_TENSION = 0x0001;
        static constexpr uint16_t FLAG_USE_CONTINUITY = 0x0002;
        static constexpr uint16_t FLAG_USE_BIAS = 0x0004;
        static constexpr uint16_t FLAG_USE_EASE_TO = 0x0008;
        static constexpr uint16_t FLAG_USE_EASE_FROM = 0x0010;
        
        // Flag check helpers
        bool UsesTension() const { return (flags & FLAG_USE_TENSION) != 0; }
        bool UsesContinuity() const { return (flags & FLAG_USE_CONTINUITY) != 0; }
        bool UsesBias() const { return (flags & FLAG_USE_BIAS) != 0; }
        bool UsesEaseTo() const { return (flags & FLAG_USE_EASE_TO) != 0; }
        bool UsesEaseFrom() const { return (flags & FLAG_USE_EASE_FROM) != 0; }
        
        // Calculate total size needed to read this key header
        size_t GetSize() const;
        
    private:
        // Helper to read spline data based on flags
        bool ReadSplineData(std::istream& stream);
    };

} // namespace Debugger3DS