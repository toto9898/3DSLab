#pragma once

#include "ObjectNodeChunks.h"  // for TrackTagChunk<T>
#include <Eigen/Dense>

namespace Debugger3DS {

    // -------------------------------------------------------------------------
    // Camera and target node container tags (KFDATA section)
    // -------------------------------------------------------------------------

    // CAMERA_NODE_TAG (0xB003) — begins an animated camera node
    class CameraNodeTagChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        std::string GetTypeName() const override { return "CAMERA_NODE_TAG"; }
    };

    // TARGET_NODE_TAG (0xB004) — begins a camera-target node
    class TargetNodeTagChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        std::string GetTypeName() const override { return "TARGET_NODE_TAG"; }
    };

    // LIGHT_NODE_TAG (0xB005) — begins an animated light node
    class LightNodeTagChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        std::string GetTypeName() const override { return "LIGHT_NODE_TAG"; }
    };

    // L_TARGET_NODE_TAG (0xB006) — begins a light-target node (spotlight)
    class LTargetNodeTagChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        std::string GetTypeName() const override { return "L_TARGET_NODE_TAG"; }
    };

    // -------------------------------------------------------------------------
    // Float animation track chunks (camera roll/fov, light hotspot/falloff)
    // -------------------------------------------------------------------------

    // FOV_TRACK_TAG (0xB023) — camera field-of-view animation (degrees)
    class FovTrackTagChunk : public TrackTagChunk<float> {
    public:
        using TrackTagChunk::TrackTagChunk;
        std::string GetTypeName() const override { return "FOV_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<float>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // ROLL_TRACK_TAG (0xB024) — camera roll/bank animation (degrees)
    class RollTrackTagChunk : public TrackTagChunk<float> {
    public:
        using TrackTagChunk::TrackTagChunk;
        std::string GetTypeName() const override { return "ROLL_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<float>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // HOT_TRACK_TAG (0xB027) — spotlight hotspot cone angle animation (degrees)
    class HotTrackTagChunk : public TrackTagChunk<float> {
    public:
        using TrackTagChunk::TrackTagChunk;
        std::string GetTypeName() const override { return "HOT_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<float>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // FALL_TRACK_TAG (0xB028) — spotlight falloff cone angle animation (degrees)
    class FallTrackTagChunk : public TrackTagChunk<float> {
    public:
        using TrackTagChunk::TrackTagChunk;
        std::string GetTypeName() const override { return "FALL_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<float>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // COL_TRACK_TAG (0xB025) — animated RGB light color
    class ColTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
    public:
        using TrackTagChunk::TrackTagChunk;
        std::string GetTypeName() const override { return "COL_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // -------------------------------------------------------------------------
    // Miscellaneous
    // -------------------------------------------------------------------------

    // OBJ_HIDDEN (0x4010) — zero-data chunk that marks a mesh as hidden by default
    class ObjHiddenChunk : public Chunk {
    public:
        using Chunk::Chunk;
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "OBJ_HIDDEN"; }
    };

} // namespace Debugger3DS
