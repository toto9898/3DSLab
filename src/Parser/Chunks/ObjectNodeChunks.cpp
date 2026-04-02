#include "ObjectNodeChunks.h"
#include "Importer.h"
#include "AnimationHeaders.h"
#include <iostream>
#include <sstream>

namespace Debugger3DS {

    bool ObjectNodeTagChunk::ReadData(Importer& importer) {
        // Begin a new object node - children will populate it
        auto& scene_ = importer.GetScene();
        scene_.objectNodes.emplace_back(std::make_shared<ObjectNode>());
        importer.SetCurrentObjectNode(scene_.objectNodes.back());
        return true;
    }

    bool ObjectNodeTagChunk::Process(Importer& importer) {
        // After all children have been read and populated the object node,
        // finalize it and add to the scene
        auto objectNode = importer.GetCurrentObjectNode();

        if (objectNode) {
            // Try to associate with existing mesh
            auto& scene_ = importer.GetScene();
            auto mesh = scene_.FindMesh(objectNode->associatedMeshName);
            if (mesh) {
                objectNode->associatedMesh = mesh;
                {
                    Eigen::Matrix4f corrected = mesh->meshMatrix;
                    if (corrected.block<3, 3>(0, 0).determinant() < 0.0f)
                        corrected.col(0).head<3>() = -corrected.col(0).head<3>();
                    objectNode->cachedMeshMatrixInverse = corrected.inverse();
                }

                // Check if bounding box is zero and calculate from mesh if needed
                bool isZeroBoundBox = (objectNode->boundingBox.min.isZero() &&
                                      objectNode->boundingBox.max.isZero());

                if (isZeroBoundBox && !mesh->vertices.empty()) {
                    auto [minPoint, maxPoint] = mesh->GetBoundingBox();
                    objectNode->boundingBox.min = minPoint;
                    objectNode->boundingBox.max = maxPoint;
                }
            }

            objectNode = nullptr;
        }


        return true;
    }

    // TrackTagChunk base class implementation
    template<typename T>
    bool TrackTagChunk<T>::ReadData(Importer& importer) {
        // Read track header using TrackHeader class
        if (!track_.header.Read(stream_)) {
            return false;
        }
        
        logging::log << GetInfo() << ", flags: " << track_.GetFlags() << std::endl;
        
        // Read keys
        track_.keys.clear();
        track_.keys.reserve(track_.GetKeyCount());
        
        for (uint32_t i = 0; i < track_.GetKeyCount(); ++i) {
            AnimationKey<T> key;
            
            // Read key header using KeyHeader::Read method
            if (!key.Read(stream_)) {
                return false;
            }
            
            // Read key-specific data using derived class implementation
            if (!ReadKeyData(key)) {
                return false;
            }
            
            track_.keys.push_back(key);
        }
        
        // Set track in importer using derived class implementation
        SetTrackInImporter(importer);
        
        return true;
    }

    template<typename T>
    std::string TrackTagChunk<T>::GetInfo() const {
        return GetTypeName() + ": " + std::to_string(track_.GetKeyCount()) + " keys";
    }

    // Explicit template instantiations
    template class TrackTagChunk<Eigen::Vector3f>;
    template class TrackTagChunk<Eigen::Vector4f>;
    template class TrackTagChunk<std::string>;
    template class TrackTagChunk<bool>;

    // PosTrackTagChunk implementation
    bool PosTrackTagChunk::ReadKeyData(AnimationKey<Eigen::Vector3f>& key) {
        // Read position vector
        return Read(key.value.x()) &&
               Read(key.value.y()) &&
               Read(key.value.z());
    }

    void PosTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto currentNode = importer.GetCurrentObjectNode()) {
            currentNode->positionTrack = track_;
        }
    }

    // RotTrackTagChunk implementation
    bool RotTrackTagChunk::ReadKeyData(AnimationKey<Eigen::Vector4f>& key) {
        // Read rotation quaternion (angle + axis in 3DS format)
        return Read(key.value.w()) &&  // Angle
               Read(key.value.x()) &&  // Axis X
               Read(key.value.y()) &&  // Axis Y
               Read(key.value.z());    // Axis Z
    }

    void RotTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto currentNode = importer.GetCurrentObjectNode()) {
            currentNode->rotationTrack = track_;
        }
    }

    // SclTrackTagChunk implementation
    bool SclTrackTagChunk::ReadKeyData(AnimationKey<Eigen::Vector3f>& key) {
        // Read scale vector
        return Read(key.value.x()) &&
               Read(key.value.y()) &&
               Read(key.value.z());
    }

    void SclTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto currentNode = importer.GetCurrentObjectNode()) {
            currentNode->scaleTrack = track_;
        }
    }

    // MorphTrackTagChunk implementation
    bool MorphTrackTagChunk::ReadKeyData(AnimationKey<std::string>& key) {
        // Read morph target name (null-terminated string)
        return Read(key.value);
    }

    void MorphTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto currentNode = importer.GetCurrentObjectNode()) {
            currentNode->morphTrack = track_;
        }
    }

    // HideTrackTagChunk implementation
    bool HideTrackTagChunk::ReadKeyData(AnimationKey<bool>& key) {
        // Hide track doesn't have additional data beyond the key header
        // If key exists, object is hidden at this frame
        key.value = true;
        return true;
    }

    void HideTrackTagChunk::SetTrackInImporter(Importer& importer) {
        if (auto currentNode = importer.GetCurrentObjectNode()) {
            currentNode->hideTrack = track_;
        }
    }

    bool NodeIdChunk::ReadData(Importer& importer) {
        // Read node ID (uint16_t)
        if (!Read(nodeId_)) {
            return false;
        }

        logging::log << GetInfo() << std::endl;

        // Set in current object node
        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->nodeId = nodeId_;
        }
        
        return true;
    }
    
    std::string NodeIdChunk::GetInfo() const {
        return "Node ID: " + std::to_string(nodeId_);
    }

    bool InstanceNameChunk::ReadData(Importer& importer) {
        // Read null-terminated string
        if (!Read(instanceName_)) {
            return false;
        }

        logging::log << GetInfo() << std::endl;

        // Set in current object node
        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->instanceName = instanceName_;
        }
        
        return true;
    }
    
    std::string InstanceNameChunk::GetInfo() const {
        return "Instance Name: " + instanceName_;
    }

    bool BoundBoxChunk::ReadData(Importer& importer) {
        // Read min point (3 floats)
        if (!Read(min_.x()) ||
            !Read(min_.y()) ||
            !Read(min_.z())) {
            return false;
        }
        
        // Read max point (3 floats)
        if (!Read(max_.x()) ||
            !Read(max_.y()) ||
            !Read(max_.z())) {
            return false;
        }

        logging::log << GetInfo() << std::endl;

        // Set in current object node
        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->boundingBox.min = min_;
            currentObjectNode->boundingBox.max = max_;
        }
        
        return true;
    }
    
    std::string BoundBoxChunk::GetInfo() const {
        std::ostringstream oss;
        oss << "BoundBox Min(" << min_.x() << ", " << min_.y() << ", " << min_.z() 
            << ") Max(" << max_.x() << ", " << max_.y() << ", " << max_.z() << ")";
        return oss.str();
    }

    bool MorphSmoothChunk::ReadData(Importer& importer) {
        // Read morph smooth value (float)
        if (!Read(morphSmooth_)) {
            return false;
        }

        logging::log << GetInfo() << std::endl;

        // Set in current object node
        if (auto currentObjectNode = importer.GetCurrentObjectNode()) {
            currentObjectNode->morphSmooth = morphSmooth_;
        }
        
        return true;
    }
    
    std::string MorphSmoothChunk::GetInfo() const {
        return "Morph Smooth: " + std::to_string(morphSmooth_);
    }

    bool PivotChunk::ReadData(Importer& importer) {
        // Read pivot point (3 floats: x, y, z)
        if (!Read(pivot_.x()) ||
            !Read(pivot_.y()) ||
            !Read(pivot_.z())) {
            return false;
        }
        
        logging::log << GetInfo() << std::endl;
        
        // Set in current object node
        if (auto currentObjectNode = importer.GetCurrentObjectNode()) { 
            currentObjectNode->pivot = pivot_;
        }
        
        return true;
    }
    
    std::string PivotChunk::GetInfo() const {
        std::ostringstream oss;
        oss << "Pivot: (" << pivot_.x() << ", " << pivot_.y() << ", " << pivot_.z() << ")";
        return oss.str();
    }

} // namespace Debugger3DS