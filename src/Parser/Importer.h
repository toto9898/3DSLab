#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <stack>
#include <Eigen/Dense>
#include "Scene.h"
#include "Chunks/ChunkFactory.h"
#include "CameraNode.h"
#include "LightNode.h"

// Forward declarations to avoid circular dependencies
namespace Debugger3DS {
    class Chunk;
    class FaceArrayChunk;
    struct ObjectNode;
    template<typename T> struct AnimationTrack;
}

namespace Debugger3DS {
    
    class Importer {
    public:
        Importer() = default;
        virtual ~Importer() = default;

        // Function to import a 3DS file
        bool Import3DS(const std::string& filename);
        
        // Get the loaded scene
        const Scene& GetScene() const { return scene_; }
        Scene& GetScene() { return scene_; }
        
        // Processing functions called by specific chunks
        void SetVersion(uint32_t version) { scene_.version = version; }
        void SetMeshVersion(uint32_t version) { scene_.meshVersion = version; }
        void SetMasterScale(float scale) { scene_.masterScale = scale; }
        
        // Object management
        void SetCurrentObjectName(const std::string& name) { currentObjectName_ = name; }
        
        // Keyframe animation management
        void SetKeyframeHeader(uint16_t revision, const std::string& filename, uint32_t animLength);
        void SetKeyframeSegment(uint32_t start, uint32_t end);
        void SetKeyframeCurrentTime(uint32_t frame);
        
        // Accessors for current object state
        const std::string& GetCurrentObjectName() const { return currentObjectName_; }
        std::shared_ptr<Mesh> GetCurrentMesh() const { return currentMesh_; }
        std::shared_ptr<Material> GetCurrentMaterial() const { return currentMaterial_; }
        void SetCurrentMesh(std::shared_ptr<Mesh> mesh) { currentMesh_ = mesh; }
        void SetCurrentMaterial(std::shared_ptr<Material> material) { currentMaterial_ = material; }
        void SetCurrentMaterialName(const std::string& name) { currentMaterialName_ = name; }
        
        // ObjectNode accessor
        ObjectNodePtr GetCurrentObjectNode() const { return currentObjectNode_; }
        void SetCurrentObjectNode(ObjectNodePtr node) { currentObjectNode_ = node; }

        // Light accessor
        std::shared_ptr<Light> GetCurrentLight() const { return currentLight_; }
        void SetCurrentLight(std::shared_ptr<Light> light) { currentLight_ = light; }

        // Camera accessor
        std::shared_ptr<Camera> GetCurrentCamera() const { return currentCamera_; }
        void SetCurrentCamera(std::shared_ptr<Camera> camera) { currentCamera_ = camera; }

        // Camera animation node accessors
        CameraNodePtr GetCurrentCameraNode() const { return currentCameraNode_; }
        void SetCurrentCameraNode(CameraNodePtr node) { currentCameraNode_ = node; }
        CameraTargetNodePtr GetCurrentCameraTargetNode() const { return currentCameraTargetNode_; }
        void SetCurrentCameraTargetNode(CameraTargetNodePtr node) { currentCameraTargetNode_ = node; }

        // Light animation node accessors
        LightNodePtr GetCurrentLightNode() const { return currentLightNode_; }
        void SetCurrentLightNode(LightNodePtr node) { currentLightNode_ = node; }
        LightTargetNodePtr GetCurrentLightTargetNode() const { return currentLightTargetNode_; }
        void SetCurrentLightTargetNode(LightTargetNodePtr node) { currentLightTargetNode_ = node; }
        
        // Factory accessor for chunk creation
        ChunkFactory& GetChunkFactory() { return chunkFactory_; }
        
    private:
        Scene scene_;
        ChunkFactory chunkFactory_;
        
    private:
        
        // Current object being processed
        std::string currentObjectName_;
        std::shared_ptr<Mesh> currentMesh_ = nullptr;
        
        // Current material being processed
        std::string currentMaterialName_;
        std::shared_ptr<Material> currentMaterial_ = nullptr;
        
        // Current object node being processed
        ObjectNodePtr currentObjectNode_ = nullptr;

        // Current light and camera being processed
        std::shared_ptr<Light> currentLight_ = nullptr;
        std::shared_ptr<Camera> currentCamera_ = nullptr;

        // Current animation nodes being processed
        CameraNodePtr           currentCameraNode_         = nullptr;
        CameraTargetNodePtr     currentCameraTargetNode_   = nullptr;
        LightNodePtr            currentLightNode_          = nullptr;
        LightTargetNodePtr      currentLightTargetNode_    = nullptr;

        // Error handling
        bool hasError_ = false;
        std::string errorMessage_;
        
        void SetError(const std::string& message) {
            hasError_ = true;
            errorMessage_ = message;
        }
    };
    
} // namespace Debugger3DS

