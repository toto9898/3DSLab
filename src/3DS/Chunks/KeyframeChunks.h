#pragma once

#include "Chunk.h"
#include <cstdint>

namespace Debugger3DS {

    // Keyframe Header Chunk (0xB00A) - Contains keyframe animation header info
    class KfHdrChunk : public Chunk {
    private:
        uint16_t revision_ = 0;
        std::string filename_;
        uint32_t animLength_ = 0;
        
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "KFHDR"; }
        std::string GetInfo() const override;
    };

    // Keyframe Segment Chunk (0xB008) - Contains segment information
    class KfSegChunk : public Chunk {
    private:
        uint32_t start_ = 0;
        uint32_t end_ = 0;
        
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "KFSEG"; }
        std::string GetInfo() const override;
    };

    // Keyframe Current Time Chunk (0xB009) - Contains current time frame
    class KfCurTimeChunk : public Chunk {
    private:
        uint32_t currentFrame_ = 0;
        
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "KFCURTIME"; }
        std::string GetInfo() const override;
    };

} // namespace Debugger3DS