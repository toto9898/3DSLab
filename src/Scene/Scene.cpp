#include <ranges>
#include <unordered_map>
#include "Scene.h"
#include "Logger.h"

namespace Debugger3DS {
    void Scene::BuildObjectNodeHierarchy()
    {
        // For R1/R2 files (no NODE_ID chunks), assign sequential IDs matching
        // positional order in the KFDATA section.
        auto nodeIdIsZero = [](const ObjectNodePtr& node) { return node->nodeId == 0; };
        if (std::all_of(objectNodes.begin(), objectNodes.end(), nodeIdIsZero)) {
            uint16_t nodeId = 0;
            for (auto&& node : objectNodes) {
                node->nodeId = nodeId++;
            }
        }

        // NODE_HDR's parent field references another node by NODE_ID.
        // For R1/R2 files without NODE_ID chunks, it references the node's
        // positional index in the KFDATA section (matches auto-assigned IDs above).
        // For R3 files, NODE_ID chunks override positional ordering.
        // 0xFFFF (65535) means no parent (root node).
        std::unordered_map<uint16_t, ObjectNodePtr> nodeById;
        for (auto& node : objectNodes) {
            nodeById.emplace(node->nodeId, node); // first occurrence wins
        }

        for (auto& node : objectNodes) {
            if (node->HasParent()) {
                auto it = nodeById.find(node->parentId);
                if (it != nodeById.end() && it->second != node) {
                    node->parentNode = it->second;
                } else {
                    logging::log << "Warning: Node '" << node->associatedMeshName
                                 << "' (ID " << node->nodeId
                                 << ") references unknown parent ID " << node->parentId
                                 << ". Treating as root." << std::endl;
                }
            }
        }
    }

    Eigen::Matrix4f Scene::GetNodeGlobalTransform(const ObjectNodePtr &node, uint32_t frame) const
    {
        if (frame == 0xFFFFFFFF)
            frame = currentFrame;
        
        Eigen::Matrix4f globalTransform = node->GetTransformAtFrame(frame);

        auto currentNode = node;
        while (currentNode->parentNode) {
            currentNode = currentNode->parentNode;
            Eigen::Matrix4f parentTransform = currentNode->GetTransformAtFrame(frame);
            globalTransform = parentTransform * globalTransform;
        }
        return globalTransform * node->cachedMeshMatrixInverse;
    }

    std::vector<uint16_t> Scene::GetDescendantNodeIds(uint16_t nodeId) const {
        std::vector<uint16_t> result;
        result.push_back(nodeId);
        // BFS over objectNodes to collect all descendants
        for (size_t i = 0; i < result.size(); ++i) {
            uint16_t current = result[i];
            for (const auto& node : objectNodes) {
                if (node->parentNode && node->parentNode->nodeId == current)
                    result.push_back(node->nodeId);
            }
        }
        return result;
    }

    // Scene class implementations
    std::shared_ptr<Mesh> Scene::FindMesh(const std::string& name) {
        for (auto& mesh : meshes) {
            if (mesh->name == name) {
                return mesh;
            }
        }
        return nullptr;
    }
    
    const std::shared_ptr<Mesh> Scene::FindMesh(const std::string& name) const {
        for (const auto& mesh : meshes) {
            if (mesh->name == name) {
                return mesh;
            }
        }
        return nullptr;
    }
    
    void Scene::PrintInfo() const {
        logging::log << "\n=== 3DS Scene Information ===\n";
        logging::log << "Version: " << version << "\n";
        logging::log << "Mesh Version: " << meshVersion << "\n";
        logging::log << "Master Scale: " << masterScale << "\n";
        logging::log << "Meshes: " << meshes.size() << "\n";
        logging::log << "Materials: " << materials.size() << "\n";
        logging::log << "Lights: " << lights.size() << "\n";
        logging::log << "Cameras: " << cameras.size() << "\n";
        logging::log << "Object Nodes: " << objectNodes.size() << "\n";
        logging::log << "Animation: " << animationLength << " frames (" 
                     << segmentStart << "-" << segmentEnd << ")\n";
        logging::log << "KF Filename: " << kfFilename << "\n";
        logging::log << "Current Frame: " << currentFrame << "\n";
        
        for (const auto& mesh : meshes) {
            logging::log << "  Mesh '" << mesh->name << "': " 
                     << mesh->vertices.size() << " vertices, "
                     << mesh->GetFaceCount() << " faces\n";
        }
        
        if (!objectNodes.empty()) {
            logging::log << "\nObject Nodes:\n";
            for (const auto& node : objectNodes) {
                logging::log << node->PrintInfo(currentFrame);
            }
        }
        
        logging::log << "============================\n\n";
    }
    
} // namespace Debugger3DS