#include "Importer.h"
#include "Chunks/Chunk.h"
#include "Chunks/FaceArrayChunk.h"
#include "Chunks/ChunkRegistration.h"
#include <iostream>
#include <fstream>

namespace Debugger3DS {
    
    bool Importer::Import3DS(const std::string& filename) {
        
        // Open the file
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            SetError("Could not open file: " + filename);
            return false;
        }
        
        // Register all chunk types
        RegisterAllChunks(file);

        logging::log << "Loading 3DS file: " << filename << std::endl;
        
        // Read the main chunk using the new Chunk method
        std::shared_ptr<Chunk> mainChunk = Chunk::CreateChunk(file, *this);
        if (!mainChunk) {
            SetError("Failed to create main chunk");
        }
        else if (mainChunk->Read(*this) == false) {
            SetError("Failed to read main chunk");
            return false;
        }
        
        // Verify it's a valid 3DS file
        if (mainChunk->GetId() != ChunkType::M3DMAGIC) {
            SetError("Not a valid 3DS file - missing M3DMAGIC chunk");
            return false;
        }
        
        logging::log << "Successfully loaded 3DS file" << std::endl;
        scene_.PrintInfo();
        
        return !hasError_;
    }
    
    void Importer::SetCurrentObjectVertices(const std::vector<Eigen::Vector3f>& vertices) {
        if (currentMesh_) {
            currentMesh_->vertices = vertices;
        }
    }
    
    void Importer::SetCurrentObjectFaces(const std::vector<Face>& faces) {
        if (currentMesh_) {
            currentMesh_->faces.clear();
            currentMesh_->faces.reserve(faces.size());
            
            for (const auto& face : faces) {
                currentMesh_->faces.emplace_back(face.a, face.b, face.c, face.flags);
            }
        }
    }
    
    void Importer::SetCurrentObjectTexCoords(const std::vector<Eigen::Vector2f>& texCoords) {
        if (currentMesh_) {
            currentMesh_->texCoords = texCoords;
        }
    }
    
    void Importer::AddMaterialGroup(const std::string& materialName, const std::vector<uint16_t>& faceIndices) {
        if (currentMesh_ && currentMaterial_) {
            // Use current material instead of searching by name
            currentMesh_->materialGroups[currentMaterial_] = faceIndices;
        }
    }
    
    void Importer::BeginMaterial(const std::string& name) {
        currentMaterialName_ = name;
        scene_.materials.emplace_back(std::make_shared<Material>(name));
        currentMaterial_ = scene_.materials.back();
        logging::log << "Processing material: " << name << std::endl;
    }
    
    void Importer::EndMaterial() {
        currentMaterial_ = nullptr;
        currentMaterialName_.clear();
    }
    
    void Importer::SetCurrentMaterialAmbient(const Eigen::Vector3f& color) {
        if (currentMaterial_) {
            currentMaterial_->ambient = color;
        }
    }
    
    void Importer::SetCurrentMaterialDiffuse(const Eigen::Vector3f& color) {
        if (currentMaterial_) {
            currentMaterial_->diffuse = color;
        }
    }
    
    void Importer::SetCurrentMaterialSpecular(const Eigen::Vector3f& color) {
        if (currentMaterial_) {
            currentMaterial_->specular = color;
        }
    }
    
    void Importer::AddLight(const std::string& name, const Eigen::Vector3f& position, const Eigen::Vector3f& color) {
        scene_.lights.emplace_back(std::make_shared<Light>(name));
        scene_.lights.back()->position = position;
        scene_.lights.back()->color = color;
    }
    
    void Importer::AddCamera(const std::string& name, const Eigen::Vector3f& position, const Eigen::Vector3f& target) {
        scene_.cameras.emplace_back(std::make_shared<Camera>(name));
        scene_.cameras.back()->position = position;
        scene_.cameras.back()->target = target;
    }
    
    void Importer::SetKeyframeHeader(uint16_t revision, const std::string& filename, uint32_t animLength) {
        scene_.kfRevision = revision;
        scene_.kfFilename = filename;
        scene_.animationLength = animLength;
    }
    
    void Importer::SetKeyframeSegment(uint32_t start, uint32_t end) {
        scene_.segmentStart = start;
        scene_.segmentEnd = end;
    }
    
    void Importer::SetKeyframeCurrentTime(uint32_t frame) {
        scene_.currentFrame = frame;
    }
    
} // namespace Debugger3DS