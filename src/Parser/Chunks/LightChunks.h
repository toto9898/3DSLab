#pragma once

#include "Chunk.h"

namespace Debugger3DS {

    // N_DIRECT_LIGHT Chunk (0x4600) - Light object inside a NAMED_OBJECT
    // ReadData reads the light position. Color is supplied by a child COLOR_F/COLOR_24 chunk.
    // Optional children: DL_SPOTLIGHT, DL_OFF.
    class NDirectLightChunk : public Chunk {
    public:
        using Chunk::Chunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "N_DIRECT_LIGHT"; }
    };

    // DL_SPOTLIGHT Chunk (0x4610) - Makes the current light a spotlight.
    // Contains target position, hotspot cone angle, and falloff cone angle.
    class DLSpotlightChunk : public Chunk {
    public:
        using Chunk::Chunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "DL_SPOTLIGHT"; }
    };

    // DL_OFF Chunk (0x4620) - Marks the current light as disabled.
    // No data payload; presence alone indicates the light is off.
    class DLOffChunk : public Chunk {
    public:
        using Chunk::Chunk;

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "DL_OFF"; }
    };

    // DL_INNER_RANGE Chunk (0x4659) - Near attenuation distance
    class DLInnerRangeChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "DL_INNER_RANGE"; }
    };

    // DL_OUTER_RANGE Chunk (0x465A) - Far attenuation distance (light fades to zero)
    class DLOuterRangeChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "DL_OUTER_RANGE"; }
    };

    // DL_MULTIPLIER Chunk (0x465B) - Light intensity multiplier
    class DLMultiplierChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "DL_MULTIPLIER"; }
    };

} // namespace Debugger3DS
