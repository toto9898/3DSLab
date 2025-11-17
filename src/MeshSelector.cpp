#include "MeshSelector.h"
#include <igl/unproject_onto_mesh.h>
#include <igl/unproject.h>
#include <iostream>
#include "3DS/SceneObjects/Mesh.h"

namespace Debugger3DS {

bool AABB::IntersectsRay(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction, double& tMin, double& tMax) const {
    // Ray-AABB intersection using slab method
    tMin = 0.0;
    tMax = std::numeric_limits<double>::max();
    
    for (int i = 0; i < 3; ++i) {
        if (std::abs(direction(i)) < 1e-8) {
            // Ray is parallel to slab, check if origin is within slab
            if (origin(i) < min(i) || origin(i) > max(i)) {
                return false;
            }
        } else {
            // Compute intersection distances to planes
            double t1 = (min(i) - origin(i)) / direction(i);
            double t2 = (max(i) - origin(i)) / direction(i);
            
            if (t1 > t2) std::swap(t1, t2);
            
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            
            if (tMin > tMax) {
                return false;
            }
        }
    }
    
    return true;
}

MeshSelector::MeshSelector(igl::opengl::glfw::Viewer& viewer)
    : viewer_(viewer), selectedObjectNode_(nullptr), selectedMeshId_(-1), currentIndex_(-1), 
      isDragging_(false), mouseDownX_(0), mouseDownY_(0) {
    
    // default selection callback prints ObjectNode info at frame 0
    selectionCallback_ = [](std::shared_ptr<Debugger3DS::ObjectNode> objectNode) {
        if (objectNode) {
            std::cout << objectNode->PrintInfo(0);

            // Print mesh matrix if available
            if (objectNode->associatedMesh) {
                std::cout << "\nMesh Matrix:" << std::endl;
                std::cout << objectNode->associatedMesh->meshMatrix << std::endl;
            }
        } else {
            std::cout << "\nNo object selected" << std::endl;
        }
    };
}

void MeshSelector::AddMesh(int dataId, ObjectNodePtr objectNode, const std::string& name, const AABB& bbox) {
    meshIds_.push_back(dataId);
    objectNodes_.push_back(objectNode);
    meshNames_.push_back(name.empty() ? "Mesh " + std::to_string(dataId) : name);
    meshBBoxes_.push_back(bbox);
    
    // Store original colors for this mesh
    originalColors_[dataId] = viewer_.data(dataId).V_material_diffuse;
}

void MeshSelector::AddMeshWithTransform(int dataId, ObjectNodePtr objectNode, const std::string& name,
                                         const Eigen::Vector3f& bboxMin, const Eigen::Vector3f& bboxMax,
                                         const Eigen::Matrix4f& transform) {
    // Transform the 8 corners of the bounding box
    Eigen::Vector3f corners[8] = {
        bboxMin,
        Eigen::Vector3f(bboxMax.x(), bboxMin.y(), bboxMin.z()),
        Eigen::Vector3f(bboxMin.x(), bboxMax.y(), bboxMin.z()),
        Eigen::Vector3f(bboxMax.x(), bboxMax.y(), bboxMin.z()),
        Eigen::Vector3f(bboxMin.x(), bboxMin.y(), bboxMax.z()),
        Eigen::Vector3f(bboxMax.x(), bboxMin.y(), bboxMax.z()),
        Eigen::Vector3f(bboxMin.x(), bboxMax.y(), bboxMax.z()),
        bboxMax
    };
    
    // Find new axis-aligned bounds after transformation
    Eigen::Vector3d transformedMin(std::numeric_limits<double>::max(),
                                   std::numeric_limits<double>::max(),
                                   std::numeric_limits<double>::max());
    Eigen::Vector3d transformedMax(std::numeric_limits<double>::lowest(),
                                   std::numeric_limits<double>::lowest(),
                                   std::numeric_limits<double>::lowest());
    
    for (int c = 0; c < 8; ++c) {
        Eigen::Vector4f homogeneous(corners[c].x(), corners[c].y(), corners[c].z(), 1.0f);
        Eigen::Vector4f transformed = transform * homogeneous;
        Eigen::Vector3d transformedCorner = transformed.head<3>().cast<double>();
        
        transformedMin = transformedMin.cwiseMin(transformedCorner);
        transformedMax = transformedMax.cwiseMax(transformedCorner);
    }
    
    AABB bbox(transformedMin, transformedMax);
    AddMesh(dataId, objectNode, name, bbox);
}

std::string MeshSelector::GetMeshName(int index) const {
    if (index >= 0 && index < meshNames_.size()) {
        return meshNames_[index];
    }
    return "";
}

void MeshSelector::SelectMesh(int dataId) {
    // Find index of this mesh
    auto it = std::find(meshIds_.begin(), meshIds_.end(), dataId);
    if (it != meshIds_.end()) {
        int index = std::distance(meshIds_.begin(), it);
        
        // Toggle selection: if clicking the same mesh, deselect it
        if (selectedMeshId_ == dataId) {
            std::cout << "Deselected: " << meshNames_[index] << std::endl;
            ClearSelection();
        } else {
            currentIndex_ = index;
            selectedMeshId_ = dataId;
            selectedObjectNode_ = objectNodes_[index];
            std::cout << "Selected: " << meshNames_[currentIndex_] 
                      << " (ID: " << selectedMeshId_ << ")" << std::endl;
            HighlightSelected();
            
            if (selectionCallback_) {
                selectionCallback_(selectedObjectNode_);
            }
        }
    }
}

void MeshSelector::EnableSelection() {
    // Setup keyboard callback for cycling through meshes
    viewer_.callback_key_pressed = [this](igl::opengl::glfw::Viewer& v, unsigned int key, int mod) {
        return OnKeyPressed(key, mod);
    };
    
    // Setup mouse callbacks for clicking on meshes
    viewer_.callback_mouse_down = [this](igl::opengl::glfw::Viewer& v, int button, int mod) {
        return OnMouseDown(button, mod);
    };
    
    viewer_.callback_mouse_up = [this](igl::opengl::glfw::Viewer& v, int button, int mod) {
        return OnMouseUp(button, mod);
    };
    
    viewer_.callback_mouse_move = [this](igl::opengl::glfw::Viewer& v, int mouse_x, int mouse_y) {
        return OnMouseMove(mouse_x, mouse_y);
    };
}

void MeshSelector::DisableSelection() {
    viewer_.callback_key_pressed = nullptr;
    viewer_.callback_mouse_down = nullptr;
    viewer_.callback_mouse_up = nullptr;
    viewer_.callback_mouse_move = nullptr;
}

void MeshSelector::SetSelectionCallback(std::function<void(ObjectNodePtr)> callback) {
    selectionCallback_ = callback;
}

void MeshSelector::HighlightSelected() {
    if (selectedMeshId_ < 0 || meshIds_.empty()) {
        return;
    }
    
    // Reset all meshes to original colors (deselect previous)
    for (size_t i = 0; i < meshIds_.size(); ++i) {
        int id = meshIds_[i];
        if (originalColors_.find(id) != originalColors_.end()) {
            viewer_.data(id).set_colors(originalColors_[id]);
        }
    }
    
    // Highlight selected mesh with yellow
    viewer_.data(selectedMeshId_).set_colors(Eigen::RowVector3d(1.0, 1.0, 0.0));
}

void MeshSelector::ClearSelection() {
    if (selectedMeshId_ >= 0) {
        // Restore original color
        if (originalColors_.find(selectedMeshId_) != originalColors_.end()) {
            viewer_.data(selectedMeshId_).set_colors(originalColors_[selectedMeshId_]);
        }
    }
    
    selectedObjectNode_ = nullptr;
    selectedMeshId_ = -1;
    currentIndex_ = -1;
}

bool MeshSelector::OnKeyPressed(unsigned int key, int modifier) {
    // N key - next mesh
    if (key == 'N' || key == 'n') {
        if (meshIds_.empty()) return false;
        
        currentIndex_ = (currentIndex_ + 1) % meshIds_.size();
        selectedMeshId_ = meshIds_[currentIndex_];
        selectedObjectNode_ = objectNodes_[currentIndex_];
        
        std::cout << "Selected: " << meshNames_[currentIndex_] 
                  << " (ID: " << selectedMeshId_ << ")" << std::endl;
        
        HighlightSelected();
        
        if (selectionCallback_) {
            selectionCallback_(selectedObjectNode_);
        }
        
        return true;
    }
    
    // P key - previous mesh
    if (key == 'P' || key == 'p') {
        if (meshIds_.empty()) return false;
        
        currentIndex_ = (currentIndex_ - 1 + meshIds_.size()) % meshIds_.size();
        selectedMeshId_ = meshIds_[currentIndex_];
        selectedObjectNode_ = objectNodes_[currentIndex_];
        
        std::cout << "Selected: " << meshNames_[currentIndex_] 
                  << " (ID: " << selectedMeshId_ << ")" << std::endl;
        
        HighlightSelected();
        
        if (selectionCallback_) {
            selectionCallback_(selectedObjectNode_);
        }
        
        return true;
    }
    
    // C key - clear selection
    if (key == 'C' || key == 'c') {
        ClearSelection();
        std::cout << "Selection cleared" << std::endl;
        return true;
    }
    
    return false;
}

bool MeshSelector::OnMouseDown(int button, int modifier) {
    // Left click - record position to detect drag vs click
    if (button == 0) { // Left mouse button
        isDragging_ = false;
        mouseDownX_ = viewer_.current_mouse_x;
        mouseDownY_ = viewer_.current_mouse_y;
    }
    
    // Always return false to allow rotation to work
    return false;
}

bool MeshSelector::OnMouseUp(int button, int modifier) {
    // Left click released - check if it was a click (not a drag)
    if (button == 0) { // Left mouse button
        if (!isDragging_) {
            // This was a click, not a drag - perform selection
            int hitMeshId = FindMeshUnderCursor();
            if (hitMeshId >= 0) {
                SelectMesh(hitMeshId);
            } else if (selectedMeshId_ >= 0) {
                // Clicked on empty space, clear selection
                ClearSelection();
                std::cout << "Selection cleared (clicked empty space)" << std::endl;
            }
        }
        isDragging_ = false;
    }
    
    // Always return false to allow default behavior
    return false;
}

bool MeshSelector::OnMouseMove(int mouse_x, int mouse_y) {
    // Detect if mouse moved significantly from mouseDown position
    const double dragThreshold = 5.0; // pixels
    double dx = mouse_x - mouseDownX_;
    double dy = mouse_y - mouseDownY_;
    double distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > dragThreshold) {
        isDragging_ = true;
    }
    
    // Always return false to allow rotation
    return false;
}

int MeshSelector::FindMeshUnderCursor() {
    // Get mouse position
    double x = viewer_.current_mouse_x;
    double y = viewer_.core().viewport(3) - viewer_.current_mouse_y;
    
    // Compute ray in world space
    Eigen::Vector3f pos1, pos2;
    igl::unproject(
        Eigen::Vector3f(x, y, 0.0f),
        viewer_.core().view,
        viewer_.core().proj,
        viewer_.core().viewport,
        pos1
    );
    igl::unproject(
        Eigen::Vector3f(x, y, 1.0f),
        viewer_.core().view,
        viewer_.core().proj,
        viewer_.core().viewport,
        pos2
    );
    
    Eigen::Vector3d rayOrigin = pos1.cast<double>();
    Eigen::Vector3d rayDir = (pos2 - pos1).normalized().cast<double>();
    
    int closestMeshId = -1;
    double minDepth = std::numeric_limits<double>::max();
    
    // Test ray intersection with each mesh
    for (size_t i = 0; i < meshIds_.size(); ++i) {
        int meshId = meshIds_[i];
        int fid;
        Eigen::Vector3f bc;
        
        // Unproject mouse coordinates onto this mesh
        bool hit = igl::unproject_onto_mesh(
            Eigen::Vector2f(x, y),
            viewer_.core().view,
            viewer_.core().proj,
            viewer_.core().viewport,
            viewer_.data(meshId).V,
            viewer_.data(meshId).F,
            fid,
            bc
        );
        
        if (hit) {
            // Calculate hit point depth
            Eigen::Vector3d hitPoint = 
                bc(0) * viewer_.data(meshId).V.row(viewer_.data(meshId).F(fid, 0)) +
                bc(1) * viewer_.data(meshId).V.row(viewer_.data(meshId).F(fid, 1)) +
                bc(2) * viewer_.data(meshId).V.row(viewer_.data(meshId).F(fid, 2));
            
            // Transform to camera space to get depth
            Eigen::Vector4d hitPoint4(hitPoint.x(), hitPoint.y(), hitPoint.z(), 1.0);
            Eigen::Vector4d camSpace = viewer_.core().view.cast<double>() * hitPoint4;
            double depth = -camSpace(2); // Negative Z is forward in camera space
            
            // Keep track of closest hit
            if (depth < minDepth) {
                minDepth = depth;
                closestMeshId = meshId;
            }
        }
    }
    
    return closestMeshId;
}

} // namespace Debugger3DS
