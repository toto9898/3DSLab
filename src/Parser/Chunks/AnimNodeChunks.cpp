#include "AnimNodeChunks.h"
#include "Importer.h"
#include "CameraNode.h"
#include "LightNode.h"

namespace Debugger3DS {

    // -------------------------------------------------------------------------
    // CAMERA_NODE_TAG (0xB003)
    // -------------------------------------------------------------------------

    bool CameraNodeTagChunk::ReadData(Importer& importer) {
        auto node = std::make_shared<CameraNode>();
        importer.GetScene().cameraNodes.push_back(node);
        importer.SetCurrentCameraNode(node);
        return true;
    }

    bool CameraNodeTagChunk::Process(Importer& importer) {
        importer.SetCurrentCameraNode(nullptr);
        return true;
    }

    // -------------------------------------------------------------------------
    // TARGET_NODE_TAG (0xB004)
    // -------------------------------------------------------------------------

    bool TargetNodeTagChunk::ReadData(Importer& importer) {
        auto node = std::make_shared<CameraTargetNode>();
        importer.GetScene().cameraTargetNodes.push_back(node);
        importer.SetCurrentCameraTargetNode(node);
        return true;
    }

    bool TargetNodeTagChunk::Process(Importer& importer) {
        importer.SetCurrentCameraTargetNode(nullptr);
        return true;
    }

    // -------------------------------------------------------------------------
    // LIGHT_NODE_TAG (0xB005)
    // -------------------------------------------------------------------------

    bool LightNodeTagChunk::ReadData(Importer& importer) {
        auto node = std::make_shared<LightNode>();
        importer.GetScene().lightNodes.push_back(node);
        importer.SetCurrentLightNode(node);
        return true;
    }

    bool LightNodeTagChunk::Process(Importer& importer) {
        importer.SetCurrentLightNode(nullptr);
        return true;
    }

    // -------------------------------------------------------------------------
    // L_TARGET_NODE_TAG (0xB006)
    // -------------------------------------------------------------------------

    bool LTargetNodeTagChunk::ReadData(Importer& importer) {
        auto node = std::make_shared<LightTargetNode>();
        importer.GetScene().lightTargetNodes.push_back(node);
        importer.SetCurrentLightTargetNode(node);
        return true;
    }

    bool LTargetNodeTagChunk::Process(Importer& importer) {
        importer.SetCurrentLightTargetNode(nullptr);
        return true;
    }

    // -------------------------------------------------------------------------
    // Float track chunks
    // -------------------------------------------------------------------------

    bool FovTrackTagChunk::ReadKeyData(AnimationKey<float>& key) {
        return Read(key.value);
    }

    void FovTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto camNode = importer.GetCurrentCameraNode())
            camNode->fovTrack = track_;
    }

    bool RollTrackTagChunk::ReadKeyData(AnimationKey<float>& key) {
        return Read(key.value);
    }

    void RollTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto camNode = importer.GetCurrentCameraNode())
            camNode->rollTrack = track_;
    }

    bool HotTrackTagChunk::ReadKeyData(AnimationKey<float>& key) {
        return Read(key.value);
    }

    void HotTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto lightNode = importer.GetCurrentLightNode())
            lightNode->hotspotTrack = track_;
    }

    bool FallTrackTagChunk::ReadKeyData(AnimationKey<float>& key) {
        return Read(key.value);
    }

    void FallTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto lightNode = importer.GetCurrentLightNode())
            lightNode->falloffTrack = track_;
    }

    // -------------------------------------------------------------------------
    // COL_TRACK_TAG (0xB025) — animated light color (RGB float per key)
    // -------------------------------------------------------------------------

    bool ColTrackTagChunk::ReadKeyData(AnimationKey<Eigen::Vector3f>& key) {
        return Read(key.value.x()) && Read(key.value.y()) && Read(key.value.z());
    }

    void ColTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto lightNode = importer.GetCurrentLightNode())
            lightNode->colorTrack = track_;
    }

    // -------------------------------------------------------------------------
    // OBJ_HIDDEN (0x4010)
    // -------------------------------------------------------------------------

    bool ObjHiddenChunk::ReadData(Importer& importer) {
        auto mesh = importer.GetCurrentMesh();
        if (mesh)
            mesh->isHidden = true;
        return true;
    }

} // namespace Debugger3DS
