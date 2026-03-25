#include "SceneTreePanel.h"
#include <imgui.h>
#include "../SceneObjects/Material.h"
#include "../SceneObjects/Light.h"
#include "../SceneObjects/Camera.h"

namespace Debugger3DS::UI {

SceneTreePanel::SceneTreePanel(const Scene& scene) : scene_(scene) {}

// -------------------------------------------------------------------------
// Public entry point — draws the full "Scene Tree" window each frame
// -------------------------------------------------------------------------
void SceneTreePanel::Draw()
{
    ImGui::SetNextWindowSize(ImVec2(320.0f, 600.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f),   ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Scene Tree")) {
        ImGui::End();
        return;
    }

    DrawSceneInfo();
    DrawMeshesSection();
    DrawMaterialsSection();
    DrawLightsSection();
    DrawCamerasSection();
    DrawObjectNodeHierarchy();

    ImGui::End();
}

// -------------------------------------------------------------------------
// Scene-level summary
// -------------------------------------------------------------------------
void SceneTreePanel::DrawSceneInfo()
{
    if (!ImGui::CollapsingHeader("Scene Info"))
        return;

    ImGui::Indent();
    ImGui::Text("Version       : %u", scene_.version);
    ImGui::Text("Mesh version  : %u", scene_.meshVersion);
    ImGui::Text("Master scale  : %.4f", scene_.masterScale);
    ImGui::Text("Ambient       : (%.2f, %.2f, %.2f)",
        scene_.ambientLight.x(), scene_.ambientLight.y(), scene_.ambientLight.z());
    ImGui::Separator();
    ImGui::Text("Animation     : %u frames  [%u – %u]",
        scene_.animationLength, scene_.segmentStart, scene_.segmentEnd);
    ImGui::Text("Current frame : %u", scene_.currentFrame);
    if (!scene_.kfFilename.empty())
        ImGui::Text("KF filename   : %s", scene_.kfFilename.c_str());
    ImGui::Separator();
    ImGui::Text("Meshes        : %zu", scene_.meshes.size());
    ImGui::Text("Materials     : %zu", scene_.materials.size());
    ImGui::Text("Lights        : %zu", scene_.lights.size());
    ImGui::Text("Cameras       : %zu", scene_.cameras.size());
    ImGui::Text("Object nodes  : %zu", scene_.objectNodes.size());
    ImGui::Unindent();
}

// -------------------------------------------------------------------------
// Meshes
// -------------------------------------------------------------------------
void SceneTreePanel::DrawMeshesSection()
{
    char header[64];
    snprintf(header, sizeof(header), "Meshes (%zu)###Meshes", scene_.meshes.size());
    if (!ImGui::CollapsingHeader(header))
        return;

    ImGui::Indent();
    for (int i = 0; i < static_cast<int>(scene_.meshes.size()); ++i)
        DrawMeshNode(i, scene_.meshes[i]);
    ImGui::Unindent();
}

void SceneTreePanel::DrawMeshNode(int index, const std::shared_ptr<Mesh>& mesh)
{
    char label[128];
    snprintf(label, sizeof(label), "[%d] %s###mesh%d", index, mesh->name.c_str(), index);

    if (!ImGui::TreeNode(label))
        return;

    char uid[32];
    snprintf(uid, sizeof(uid), "m%d", index);
    DrawMeshDetails(mesh, uid);

    ImGui::TreePop();
}

void SceneTreePanel::DrawMeshDetails(const std::shared_ptr<Mesh>& mesh, const char* uid)
{
    ImGui::Text("Vertices : %zu", mesh->vertices.size());
    ImGui::Text("Faces    : %zu", mesh->faces.size());
    ImGui::Text("TexCoords: %zu", mesh->texCoords.size());

    if (!mesh->vertices.empty()) {
        char bbLabel[64];
        snprintf(bbLabel, sizeof(bbLabel), "Bounding Box###bbox_%s", uid);
        auto [bmin, bmax] = mesh->GetBoundingBox();
        if (ImGui::TreeNode(bbLabel)) {
            ImGui::Text("Min: (%.2f, %.2f, %.2f)", bmin.x(), bmin.y(), bmin.z());
            ImGui::Text("Max: (%.2f, %.2f, %.2f)", bmax.x(), bmax.y(), bmax.z());
            ImGui::TreePop();
        }
    }

    if (!mesh->materialGroups.empty()) {
        char matLabel[64];
        snprintf(matLabel, sizeof(matLabel), "Materials (%zu)###mats_%s",
                 mesh->materialGroups.size(), uid);
        if (ImGui::TreeNode(matLabel)) {
            for (const auto& [mat, faceIndices] : mesh->materialGroups) {
                if (mat)
                    ImGui::BulletText("%-24s  (%zu faces)", mat->name.c_str(), faceIndices.size());
            }
            ImGui::TreePop();
        }
    }

    char matrixLabel[64];
    snprintf(matrixLabel, sizeof(matrixLabel), "Mesh Matrix###mtx_%s", uid);
    if (ImGui::TreeNode(matrixLabel)) {
        const Eigen::Matrix4f& m = mesh->meshMatrix;
        for (int row = 0; row < 4; ++row)
            ImGui::Text("  [ %8.3f  %8.3f  %8.3f  %8.3f ]",
                m(row, 0), m(row, 1), m(row, 2), m(row, 3));
        ImGui::TreePop();
    }
}

// -------------------------------------------------------------------------
// Materials
// -------------------------------------------------------------------------
void SceneTreePanel::DrawMaterialsSection()
{
    char header[64];
    snprintf(header, sizeof(header), "Materials (%zu)###Materials", scene_.materials.size());
    if (!ImGui::CollapsingHeader(header))
        return;

    ImGui::Indent();
    for (int i = 0; i < static_cast<int>(scene_.materials.size()); ++i)
        DrawMaterialNode(i, scene_.materials[i]);
    ImGui::Unindent();
}

void SceneTreePanel::DrawMaterialNode(int index, const std::shared_ptr<Material>& mat)
{
    char label[128];
    snprintf(label, sizeof(label), "[%d] %s###mat%d", index, mat->name.c_str(), index);

    if (!ImGui::TreeNode(label))
        return;

    ImVec4 ambient (mat->ambient.x(),  mat->ambient.y(),  mat->ambient.z(),  1.0f);
    ImVec4 diffuse (mat->diffuse.x(),  mat->diffuse.y(),  mat->diffuse.z(),  1.0f);
    ImVec4 specular(mat->specular.x(), mat->specular.y(), mat->specular.z(), 1.0f);

    ImGui::ColorButton("##amb", ambient,  ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine(); ImGui::Text("Ambient  (%.2f, %.2f, %.2f)",
        mat->ambient.x(), mat->ambient.y(), mat->ambient.z());

    ImGui::ColorButton("##dif", diffuse,  ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine(); ImGui::Text("Diffuse  (%.2f, %.2f, %.2f)",
        mat->diffuse.x(), mat->diffuse.y(), mat->diffuse.z());

    ImGui::ColorButton("##spc", specular, ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine(); ImGui::Text("Specular (%.2f, %.2f, %.2f)",
        mat->specular.x(), mat->specular.y(), mat->specular.z());

    ImGui::Text("Shininess    : %.2f  (%.0f%%)", mat->shininess, mat->shininessPercent);
    ImGui::Text("Transparency : %.2f", mat->transparency);
    ImGui::Text("Self-illum   : %.2f", mat->selfIllumination);

    const char* shadingNames[] = {"Flat", "Gouraud", "Phong", "Metal"};
    const char* shadingStr = (mat->shadingType < 4) ? shadingNames[mat->shadingType] : "Unknown";
    ImGui::Text("Shading      : %s (%u)", shadingStr, mat->shadingType);

    if (!mat->textureMap.empty())
        ImGui::Text("Texture      : %s", mat->textureMap.c_str());

    ImGui::TreePop();
}

// -------------------------------------------------------------------------
// Lights
// -------------------------------------------------------------------------
void SceneTreePanel::DrawLightsSection()
{
    char header[64];
    snprintf(header, sizeof(header), "Lights (%zu)###Lights", scene_.lights.size());
    if (!ImGui::CollapsingHeader(header))
        return;

    ImGui::Indent();
    for (int i = 0; i < static_cast<int>(scene_.lights.size()); ++i)
        DrawLightNode(i, scene_.lights[i]);
    ImGui::Unindent();
}

void SceneTreePanel::DrawLightNode(int index, const std::shared_ptr<Light>& light)
{
    char label[128];
    snprintf(label, sizeof(label), "[%d] %s  (%s)###light%d",
        index, light->name.c_str(), light->isSpotlight ? "Spot" : "Omni", index);

    if (!ImGui::TreeNode(label))
        return;

    ImGui::Text("Position : (%.2f, %.2f, %.2f)",
        light->position.x(), light->position.y(), light->position.z());

    ImVec4 col(light->color.x(), light->color.y(), light->color.z(), 1.0f);
    ImGui::ColorButton("##lc", col, ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine(); ImGui::Text("Color (%.2f, %.2f, %.2f)",
        light->color.x(), light->color.y(), light->color.z());

    ImGui::Text("State    : %s", light->isOn ? "On" : "Off");

    if (light->isSpotlight) {
        ImGui::Text("Target   : (%.2f, %.2f, %.2f)",
            light->target.x(), light->target.y(), light->target.z());
        ImGui::Text("Hotspot  : %.1f°  Falloff: %.1f°",
            light->hotspotAngle, light->falloffAngle);
    }

    ImGui::TreePop();
}

// -------------------------------------------------------------------------
// Cameras
// -------------------------------------------------------------------------
void SceneTreePanel::DrawCamerasSection()
{
    char header[64];
    snprintf(header, sizeof(header), "Cameras (%zu)###Cameras", scene_.cameras.size());
    if (!ImGui::CollapsingHeader(header))
        return;

    ImGui::Indent();
    for (int i = 0; i < static_cast<int>(scene_.cameras.size()); ++i)
        DrawCameraNode(i, scene_.cameras[i]);
    ImGui::Unindent();
}

void SceneTreePanel::DrawCameraNode(int index, const std::shared_ptr<Camera>& cam)
{
    char label[128];
    snprintf(label, sizeof(label), "[%d] %s###cam%d", index, cam->name.c_str(), index);

    if (!ImGui::TreeNode(label))
        return;

    ImGui::Text("Position     : (%.2f, %.2f, %.2f)",
        cam->position.x(), cam->position.y(), cam->position.z());
    ImGui::Text("Target       : (%.2f, %.2f, %.2f)",
        cam->target.x(), cam->target.y(), cam->target.z());
    ImGui::Text("Focal length : %.2f mm", cam->focalLength);
    ImGui::Text("Bank angle   : %.2f°", cam->bankAngle);

    ImGui::TreePop();
}

// -------------------------------------------------------------------------
// Object node hierarchy (parent-child tree)
// -------------------------------------------------------------------------
void SceneTreePanel::DrawObjectNodeHierarchy()
{
    char header[64];
    snprintf(header, sizeof(header), "Object Nodes (%zu)###ObjectNodes",
             scene_.objectNodes.size());
    if (!ImGui::CollapsingHeader(header))
        return;

    ImGui::Indent();
    // Render only root nodes; children are drawn recursively
    for (const auto& node : scene_.objectNodes) {
        if (node->parentNode == nullptr)
            DrawObjectNode(node, scene_.objectNodes);
    }
    ImGui::Unindent();
}

void SceneTreePanel::DrawObjectNode(const ObjectNodePtr& node,
                                    const std::vector<ObjectNodePtr>& allNodes)
{
    const std::string& displayName = !node->instanceName.empty()
        ? node->instanceName
        : node->associatedMeshName;

    char label[256];
    snprintf(label, sizeof(label), "[ID %u] %s###node%u",
        node->nodeId,
        displayName.empty() ? "(unnamed)" : displayName.c_str(),
        node->nodeId);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                             | ImGuiTreeNodeFlags_SpanAvailWidth;

    // Collect children for recursive rendering
    std::vector<ObjectNodePtr> children;
    for (const auto& n : allNodes) {
        if (n->parentNode.get() == node.get())
            children.push_back(n);
    }

    bool open = ImGui::TreeNodeEx(label, flags);

    if (open) {
        // Mesh link — expandable like a Meshes section entry
        if (node->associatedMesh) {
            char meshLabel[256];
            snprintf(meshLabel, sizeof(meshLabel),
                "Mesh: %s  (%zu verts, %zu faces)###nodemesh%u",
                displayName.c_str(),
                node->associatedMesh->vertices.size(),
                node->associatedMesh->faces.size(),
                node->nodeId);
            if (ImGui::TreeNode(meshLabel)) {
                char uid[32];
                snprintf(uid, sizeof(uid), "n%u", node->nodeId);
                DrawMeshDetails(node->associatedMesh, uid);
                ImGui::TreePop();
            }
        } else if (displayName.empty() || displayName == "$$$DUMMY") {
            ImGui::Text("Mesh     : %s  (not resolved)", node->associatedMeshName.c_str());
        }

        // Pivot
        ImGui::Text("Pivot    : (%.2f, %.2f, %.2f)",
            node->pivot.x(), node->pivot.y(), node->pivot.z());

        // Bounding box
        ImGui::Text("BBox min : (%.2f, %.2f, %.2f)",
            node->boundingBox.min.x(), node->boundingBox.min.y(), node->boundingBox.min.z());
        ImGui::Text("BBox max : (%.2f, %.2f, %.2f)",
            node->boundingBox.max.x(), node->boundingBox.max.y(), node->boundingBox.max.z());

        // Animation tracks — show interpolated values at the active frame
        char tracksLabel[64];
        snprintf(tracksLabel, sizeof(tracksLabel), "Animation Tracks###tracks%u", node->nodeId);
        if (ImGui::TreeNode(tracksLabel)) {
            const uint32_t frame = scene_.currentFrame;

            if (node->positionTrack.HasKeys()) {
                Eigen::Vector3f pos = node->positionTrack.GetValueAtFrame(frame);
                ImGui::Text("Position : (%.2f, %.2f, %.2f)  [%u key(s)]",
                    pos.x(), pos.y(), pos.z(), node->positionTrack.GetKeyCount());
            } else {
                ImGui::TextDisabled("Position : —");
            }

            if (node->rotationTrack.HasKeys()) {
                Eigen::Vector4f rot = node->rotationTrack.GetValueAtFrame(frame);
                // rot.xyz = axis, rot.w = angle (radians)
                float angleDeg = rot.w() * (180.0f / 3.14159265f);
                ImGui::Text("Rotation : %.1f\xc2\xb0 @ (%.2f, %.2f, %.2f)  [%u key(s)]",
                    angleDeg, rot.x(), rot.y(), rot.z(), node->rotationTrack.GetKeyCount());
            } else {
                ImGui::TextDisabled("Rotation : —");
            }

            if (node->scaleTrack.HasKeys()) {
                Eigen::Vector3f scl = node->scaleTrack.GetValueAtFrame(frame);
                ImGui::Text("Scale    : (%.3f, %.3f, %.3f)  [%u key(s)]",
                    scl.x(), scl.y(), scl.z(), node->scaleTrack.GetKeyCount());
            } else {
                ImGui::TextDisabled("Scale    : —");
            }

            if (node->morphTrack.HasKeys()) {
                std::string morph = node->morphTrack.GetValueAtFrame(frame);
                ImGui::Text("Morph    : \"%s\"  [%u key(s)]",
                    morph.c_str(), node->morphTrack.GetKeyCount());
            } else {
                ImGui::TextDisabled("Morph    : —");
            }

            if (node->hideTrack.HasKeys()) {
                bool hidden = node->hideTrack.GetValueAtFrame(frame);
                ImGui::Text("Hide     : %s  [%u key(s)]",
                    hidden ? "true" : "false", node->hideTrack.GetKeyCount());
            } else {
                ImGui::TextDisabled("Hide     : —");
            }

            ImGui::TreePop();
        }

        // Recurse into children
        for (const auto& child : children)
            DrawObjectNode(child, allNodes);

        ImGui::TreePop();
    }
}

} // namespace Debugger3DS::UI
