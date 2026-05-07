#pragma once

#include "Chunk.h"

namespace Debugger3DS {

    // N_CAMERA Chunk (0x4700) - Camera object inside a NAMED_OBJECT.
    // All camera data is stored inline: position, target, bank angle, focal length.
    class NCameraChunk : public Chunk {
    public:
        using Chunk::Chunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "N_CAMERA"; }
    };

    // CAM_RANGES Chunk (0x4720) - Camera near/far clip plane distances
    class CamRangesChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "CAM_RANGES"; }
    };

} // namespace Debugger3DS
