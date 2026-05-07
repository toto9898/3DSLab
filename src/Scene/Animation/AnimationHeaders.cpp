#include "AnimationHeaders.h"
#include <sstream>
#include <iomanip>

namespace Debugger3DS::Scene {

    // TrackHeader implementation
    bool TrackHeader::Read(std::istream& stream) {
        // Read flags (2 bytes) separately due to alignment padding before uint32_t fields
        stream.read(reinterpret_cast<char*>(&flags), sizeof(flags));
        if (!stream.good()) return false;

        // Bulk read the three contiguous uint32_t fields (12 bytes) in one call
        stream.read(reinterpret_cast<char*>(&unknown1), 3 * sizeof(uint32_t));
        if (!stream.good()) return false;
        
        return true;
    }

    std::string TrackHeader::GetInfo() const {
        std::ostringstream oss;
        oss << "TrackHeader: flags=0x" << std::hex << flags << std::dec 
            << ", keyCount=" << keyCount;
        
        if (IsLooped()) oss << " [LOOP]";
        if (IsXLocked()) oss << " [LOCK_X]";
        if (IsYLocked()) oss << " [LOCK_Y]";
        if (IsZLocked()) oss << " [LOCK_Z]";
        if (IsXUnlinked()) oss << " [UNLINK_X]";
        if (IsYUnlinked()) oss << " [UNLINK_Y]";
        if (IsZUnlinked()) oss << " [UNLINK_Z]";
        
        return oss.str();
    }

    bool TrackHeader::IsValid() const {
        return keyCount < 100000;
    }

    // KeyHeader implementation  
    bool KeyHeader::Read(std::istream& stream) {
        stream.read(reinterpret_cast<char*>(&frame), sizeof(frame));
        if (!stream.good()) return false;

        stream.read(reinterpret_cast<char*>(&flags), sizeof(flags));
        if (!stream.good()) return false;

        return ReadSplineData(stream);
    }

    bool KeyHeader::ReadSplineData(std::istream& stream) {
        if (UsesTension()) {
            stream.read(reinterpret_cast<char*>(&tension), sizeof(tension));
            if (!stream.good()) return false;
        }
        
        if (UsesContinuity()) {
            stream.read(reinterpret_cast<char*>(&continuity), sizeof(continuity));
            if (!stream.good()) return false;
        }
        
        if (UsesBias()) {
            stream.read(reinterpret_cast<char*>(&bias), sizeof(bias));
            if (!stream.good()) return false;
        }
        
        if (UsesEaseTo()) {
            stream.read(reinterpret_cast<char*>(&easeTo), sizeof(easeTo));
            if (!stream.good()) return false;
        }
        
        if (UsesEaseFrom()) {
            stream.read(reinterpret_cast<char*>(&easeFrom), sizeof(easeFrom));
            if (!stream.good()) return false;
        }
        
        return true;
    }

    std::string KeyHeader::GetInfo() const {
        std::ostringstream oss;
        oss << "KeyHeader: frame=" << frame << ", flags=0x" << std::hex << flags << std::dec;
        
        if (UsesTension()) oss << ", tension=" << tension;
        if (UsesContinuity()) oss << ", continuity=" << continuity;
        if (UsesBias()) oss << ", bias=" << bias;
        if (UsesEaseTo()) oss << ", easeTo=" << easeTo;
        if (UsesEaseFrom()) oss << ", easeFrom=" << easeFrom;
        
        return oss.str();
    }

    bool KeyHeader::IsValid() const {
        return frame < 1000000;
    }

    size_t KeyHeader::GetSize() const {
        size_t size = 6;  // Base size (frame + flags)
        
        if (UsesTension()) size += 4;
        if (UsesContinuity()) size += 4;
        if (UsesBias()) size += 4;
        if (UsesEaseTo()) size += 4;
        if (UsesEaseFrom()) size += 4;
        
        return size;
    }

} // namespace Debugger3DS::Scene