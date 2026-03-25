#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <stack>
#include <Eigen/Dense>
#include "Scene.h"
#include "Chunks/ChunkFactory.h"

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
        void SetAmbientLight(const Eigen::Vector3f& color) { scene_.ambientLight = color; }
        
        // Object management
        void SetCurrentObjectName(const std::string& name) { currentObjectName_ = name; }
        void SetCurrentObjectVertices(const std::vector<Eigen::Vector3f>& vertices);
        void SetCurrentObjectFaces(const std::vector<Face>& faces);
        void SetCurrentObjectTexCoords(const std::vector<Eigen::Vector2f>& texCoords);
        void AddMaterialGroup(const std::string& materialName, const std::vector<uint16_t>& faceIndices);
        
        // Material management
        void BeginMaterial(const std::string& name);
        void EndMaterial();
        void SetCurrentMaterialAmbient(const Eigen::Vector3f& color);
        void SetCurrentMaterialDiffuse(const Eigen::Vector3f& color);
        void SetCurrentMaterialSpecular(const Eigen::Vector3f& color);
        
        // Light management
        void AddLight(const std::string& name, const Eigen::Vector3f& position, const Eigen::Vector3f& color);
        
        // Camera management
        void AddCamera(const std::string& name, const Eigen::Vector3f& position, const Eigen::Vector3f& target);
        
        // Keyframe animation management
        void SetKeyframeHeader(uint16_t revision, const std::string& filename, uint32_t animLength);
        void SetKeyframeSegment(uint32_t start, uint32_t end);
        void SetKeyframeCurrentTime(uint32_t frame);
        
        // Accessors for current object state
        const std::string& GetCurrentObjectName() const { return currentObjectName_; }
        const std::string& GetCurrentMaterialName() const { return currentMaterialName_; }
        std::shared_ptr<Mesh> GetCurrentMesh() const { return currentMesh_; }
        std::shared_ptr<Material> GetCurrentMaterial() const { return currentMaterial_; }
        void SetCurrentMesh(std::shared_ptr<Mesh> mesh) { currentMesh_ = mesh; }
        void SetCurrentMaterial(std::shared_ptr<Material> material) { currentMaterial_ = material; }
        void SetCurrentMaterialName(const std::string& name) { currentMaterialName_ = name; }
        
        // ObjectNode accessor
        ObjectNodePtr GetCurrentObjectNode() const { return currentObjectNode_; }
        void SetCurrentObjectNode(ObjectNodePtr node) { currentObjectNode_ = node; }
        
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
        
        // Error handling
        bool hasError_ = false;
        std::string errorMessage_;
        
        void SetError(const std::string& message) {
            hasError_ = true;
            errorMessage_ = message;
        }
    };
    
} // namespace Debugger3DS
