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
namespace Debugger3DS::Parser::Chunks { class Chunk; class FaceArrayChunk; }
namespace Debugger3DS::Scene { struct ObjectNode; template<typename T> struct AnimationTrack; }

namespace Debugger3DS::Parser {

// Pull in Scene and Chunk types used throughout this header
using Debugger3DS::Scene::Scene;
using Debugger3DS::Scene::Mesh;
using Debugger3DS::Scene::Material;
using Debugger3DS::Scene::Light;
using Debugger3DS::Scene::Camera;
using Debugger3DS::Scene::ObjectNode;
using Debugger3DS::Scene::ObjectNodePtr;
using Debugger3DS::Scene::CameraNode;
using Debugger3DS::Scene::CameraNodePtr;
using Debugger3DS::Scene::CameraTargetNode;
using Debugger3DS::Scene::CameraTargetNodePtr;
using Debugger3DS::Scene::LightNode;
using Debugger3DS::Scene::LightNodePtr;
using Debugger3DS::Scene::LightTargetNode;
using Debugger3DS::Scene::LightTargetNodePtr;
using Debugger3DS::Parser::Chunks::ChunkFactory;

/// @brief Parses a binary 3DS file and populates a Scene.
///
/// The importer owns a Scene and a ChunkFactory.  Individual chunk
/// implementations call the public setters below to write parsed data into
/// the scene while the chunk tree is being walked.
///
/// @par Typical usage
/// @code
/// Importer imp;
/// if (imp.Import3DS("model.3ds")) {
///     const Scene& scene = imp.GetScene();
///     // use scene ...
/// }
/// @endcode
class Importer {
public:
    Importer() = default;
    virtual ~Importer() = default;

    /// @brief Load and parse a 3DS file.
    /// @param filename Path to the .3ds file.
    /// @return @c true on success; errors are reported via @c std::cerr.
    bool Import3DS(const std::string& filename);

    const Scene& GetScene() const { return scene_; } ///< Read-only access to the parsed scene.
    Scene&       GetScene()       { return scene_; } ///< Mutable access to the parsed scene.

    /// @name Scene property setters (called by version/scale chunks)
    /// @{
    void SetVersion(uint32_t version)    { scene_.version = version; }      ///< Set the 3DS file version.
    void SetMeshVersion(uint32_t version){ scene_.meshVersion = version; }   ///< Set the mesh version.
    void SetMasterScale(float scale)     { scene_.masterScale = scale; }     ///< Set the master scene scale.
    /// @}

    void SetCurrentObjectName(const std::string& name) { currentObjectName_ = name; } ///< Record the name of the NAMED_OBJECT currently being parsed.

    /// @name Keyframe animation setters (called by KFDATA chunks)
    /// @{
    void SetKeyframeHeader(uint16_t revision, const std::string& filename, uint32_t animLength); ///< Set the KF header from KF_HDR.
    void SetKeyframeSegment(uint32_t start, uint32_t end); ///< Set the active frame range from KF_SEG.
    void SetKeyframeCurrentTime(uint32_t frame); ///< Set the current playback frame from KF_CURTIME.
    /// @}

    /// @name Current-object accessors (used by chunk processors)
    /// @{
    const std::string& GetCurrentObjectName() const { return currentObjectName_; } ///< Name of the object currently being parsed.
    std::shared_ptr<Mesh>     GetCurrentMesh()     const { return currentMesh_; }         ///< Current mesh being populated.
    std::shared_ptr<Material> GetCurrentMaterial() const { return currentMaterial_; }     ///< Current material being populated.
    void SetCurrentMesh(std::shared_ptr<Mesh> mesh)          { currentMesh_ = mesh; }          ///< Set the current mesh.
    void SetCurrentMaterial(std::shared_ptr<Material> material) { currentMaterial_ = material; } ///< Set the current material.
    void SetCurrentMaterialName(const std::string& name)     { currentMaterialName_ = name; }  ///< Set the current material name.
    /// @}

    /// @name Object-node accessors
    /// @{
    ObjectNodePtr GetCurrentObjectNode() const         { return currentObjectNode_; }  ///< Current OBJECT_NODE_TAG being populated.
    void SetCurrentObjectNode(ObjectNodePtr node)       { currentObjectNode_ = node; }  ///< Set the current object node.
    /// @}

    /// @name Light/camera accessors
    /// @{
    std::shared_ptr<Light>  GetCurrentLight()  const { return currentLight_; }          ///< Current light being populated.
    void SetCurrentLight(std::shared_ptr<Light> light) { currentLight_ = light; }       ///< Set the current light.
    std::shared_ptr<Camera> GetCurrentCamera() const { return currentCamera_; }         ///< Current camera being populated.
    void SetCurrentCamera(std::shared_ptr<Camera> camera) { currentCamera_ = camera; }  ///< Set the current camera.
    /// @}

    /// @name Animated camera-node accessors
    /// @{
    CameraNodePtr       GetCurrentCameraNode()       const { return currentCameraNode_; }         ///< Current CAMERA_NODE_TAG being populated.
    void SetCurrentCameraNode(CameraNodePtr node)          { currentCameraNode_ = node; }          ///< Set the current camera node.
    CameraTargetNodePtr GetCurrentCameraTargetNode() const { return currentCameraTargetNode_; }   ///< Current TARGET_NODE_TAG for cameras.
    void SetCurrentCameraTargetNode(CameraTargetNodePtr node) { currentCameraTargetNode_ = node; } ///< Set the current camera target node.
    /// @}

    /// @name Animated light-node accessors
    /// @{
    LightNodePtr       GetCurrentLightNode()       const { return currentLightNode_; }         ///< Current LIGHT_NODE_TAG being populated.
    void SetCurrentLightNode(LightNodePtr node)          { currentLightNode_ = node; }          ///< Set the current light node.
    LightTargetNodePtr GetCurrentLightTargetNode() const { return currentLightTargetNode_; }   ///< Current L_TARGET_NODE_TAG for spotlights.
    void SetCurrentLightTargetNode(LightTargetNodePtr node) { currentLightTargetNode_ = node; } ///< Set the current light target node.
    /// @}

    /// @brief Access the chunk factory used for parsing.
    ChunkFactory& GetChunkFactory() { return chunkFactory_; }

private:
    Scene scene_;
    ChunkFactory chunkFactory_;

    // Current object being processed
    std::string currentObjectName_;
    std::shared_ptr<Mesh> currentMesh_ = nullptr;

    // Current material being processed
    std::string currentMaterialName_;
    std::shared_ptr<Material> currentMaterial_ = nullptr;

    // Current object node being processed
    ObjectNodePtr currentObjectNode_ = nullptr;

    // Current light and camera being processed
    std::shared_ptr<Light>  currentLight_  = nullptr;
    std::shared_ptr<Camera> currentCamera_ = nullptr;

    // Current animation nodes being processed
    CameraNodePtr       currentCameraNode_       = nullptr;
    CameraTargetNodePtr currentCameraTargetNode_ = nullptr;
    LightNodePtr        currentLightNode_        = nullptr;
    LightTargetNodePtr  currentLightTargetNode_  = nullptr;
};

} // namespace Debugger3DS::Parser
