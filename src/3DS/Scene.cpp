#include "Scene.h"
#include "../Logger.h"

namespace Debugger3DS {
    
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
                     << mesh->faces.size() << " faces\n";
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