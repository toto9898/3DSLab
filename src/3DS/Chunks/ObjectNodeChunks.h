#pragma once

#include "Chunk.h"
#include "ContainerChunks.h"
#include "../SceneObjects/ObjectNode.h"
#include <cstdint>
#include <Eigen/Dense>
#include <vector>

namespace Debugger3DS {

    // OBJECT_NODE_TAG Chunk (0xB002) - Container for object node data
    class ObjectNodeTagChunk : public Chunk {
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        bool ReadData(Importer& importer) override;
        bool Process(Importer& importer) override;
        std::string GetTypeName() const override { return "OBJECT_NODE_TAG"; }
    };

    // NODE_ID Chunk (0xB030) - Contains node ID
    class NodeIdChunk : public Chunk {
    private:
        uint16_t nodeId_ = 0;
        
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "NODE_ID"; }
        std::string GetInfo() const override;
    };

    // INSTANCE_NAME Chunk (0xB031) - Contains instance name  
    class InstanceNameChunk : public Chunk {
    private:
        std::string instanceName_;
        
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "INSTANCE_NAME"; }
        std::string GetInfo() const override;
    };

    // BOUNDBOX Chunk (0xB014) - Contains bounding box
    class BoundBoxChunk : public Chunk {
    private:
        Eigen::Vector3f min_;
        Eigen::Vector3f max_;
        
    public:
        using Chunk::Chunk;  // Inherit constructor 
        
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "BOUNDBOX"; }
        std::string GetInfo() const override;
    };

    // Base class for all track tag chunks
    template<typename T>
    class TrackTagChunk : public Chunk {
    protected:
        AnimationTrack<T> track_;
        
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override final;
        std::string GetInfo() const override;
        
        // Pure virtual function for reading key-specific data
        virtual bool ReadKeyData(AnimationKey<T>& key) = 0;
        
        // Pure virtual function for setting track in importer
        virtual void SetTrackInImporter(Importer& importer) = 0;
        
        const AnimationTrack<T>& GetTrack() const { return track_; }
    };

    // POS_TRACK_TAG Chunk (0xB020) - Position animation track
    class PosTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
    public:
        using TrackTagChunk::TrackTagChunk;  // Inherit constructor

        std::string GetTypeName() const override { return "POS_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // ROT_TRACK_TAG Chunk (0xB021) - Rotation animation track
    class RotTrackTagChunk : public TrackTagChunk<Eigen::Vector4f> {
    public:
        using TrackTagChunk::TrackTagChunk;  // Inherit constructor
        
        std::string GetTypeName() const override { return "ROT_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<Eigen::Vector4f>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // SCL_TRACK_TAG Chunk (0xB022) - Scale animation track
    class SclTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
    public:
        using TrackTagChunk::TrackTagChunk;  // Inherit constructor
        
        std::string GetTypeName() const override { return "SCL_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // MORPH_TRACK_TAG Chunk (0xB026) - Morph animation track
    class MorphTrackTagChunk : public TrackTagChunk<std::string> {
    public:
        using TrackTagChunk::TrackTagChunk;  // Inherit constructor
        
        std::string GetTypeName() const override { return "MORPH_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<std::string>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // HIDE_TRACK_TAG Chunk (0xB029) - Hide animation track
    class HideTrackTagChunk : public TrackTagChunk<bool> {
    public:
        using TrackTagChunk::TrackTagChunk;  // Inherit constructor
        
        std::string GetTypeName() const override { return "HIDE_TRACK_TAG"; }
        bool ReadKeyData(AnimationKey<bool>& key) override;
        void SetTrackInImporter(Importer& importer) override;
    };

    // MORPH_SMOOTH Chunk (0xB032) - Morph smoothing value
    class MorphSmoothChunk : public Chunk {
    private:
        float morphSmooth_ = 0.0f;
        
    public:
        using Chunk::Chunk;  // Inherit constructor

        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "MORPH_SMOOTH"; }
        std::string GetInfo() const override;
    };

    // PIVOT Chunk (0xB013) - Pivot point
    class PivotChunk : public Chunk {
    private:
        Eigen::Vector3f pivot_;
        
    public:
        using Chunk::Chunk;  // Inherit constructor
        
        bool ReadData(Importer& importer) override;
        std::string GetTypeName() const override { return "PIVOT"; }
        std::string GetInfo() const override;
        
        const Eigen::Vector3f& GetPivot() const { return pivot_; }
    };

} // namespace Debugger3DS