#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include "SceneObjects/Material.h"
#include "SceneObjects/Mesh.h"
#include "SceneObjects/Light.h"
#include "SceneObjects/Camera.h"
#include "SceneObjects/ObjectNode.h"

namespace Debugger3DS {
    
    // Complete 3DS scene data
    class Scene {
    public:
        uint32_t version = 0;
        uint32_t meshVersion = 0;
        float masterScale = 1.0f;
        Eigen::Vector3f ambientLight = Eigen::Vector3f(0.1f, 0.1f, 0.1f);
        
        // Keyframe animation data
        uint16_t kfRevision = 0;
        std::string kfFilename;
        uint32_t animationLength = 100;
        uint32_t segmentStart = 0;
        uint32_t segmentEnd = 100;
        uint32_t currentFrame = 0;
        
        std::vector<std::shared_ptr<Material>>  materials;
        std::vector<std::shared_ptr<Mesh>>      meshes;
        std::vector<std::shared_ptr<Light>>     lights;
        std::vector<std::shared_ptr<Camera>>    cameras;
        std::vector<ObjectNodePtr> objectNodes;
        
        void BuildObjectNodeHierarchy();
        Eigen::Matrix4f GetNodeGlobalTransform(const ObjectNodePtr& node, uint32_t frame = 0xFFFFFFFF) const;
        
        // Collect nodeId + all descendant nodeIds (BFS)
        std::vector<uint16_t> GetDescendantNodeIds(uint16_t nodeId) const;
        
        // Helper functions
        std::shared_ptr<Mesh> FindMesh(const std::string& name);
        const std::shared_ptr<Mesh> FindMesh(const std::string& name) const;
        
        // Print scene information
        void PrintInfo() const;
    };
    
} // namespace Debugger3DS
