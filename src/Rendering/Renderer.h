#pragma once

#include <bgfx/bgfx.h>
#include <Eigen/Dense>
#include <vector>
#include <cstdint>

namespace Debugger3DS {

// Per-vertex data for line rendering (position + ABGR color, no normal)
struct PosColorVertex {
    float x, y, z;
    uint32_t abgr;

    static void Init() {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }
    static bgfx::VertexLayout layout;
};

// Per-vertex data for mesh rendering (position + normal + ABGR color)
struct PosNormalColorVertex {
    float x, y, z;
    float nx, ny, nz;
    uint32_t abgr;

    static void Init() {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .end();
    }
    static bgfx::VertexLayout layout;
};

// Handle returned when a mesh is uploaded to the GPU
struct GpuMesh {
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
    uint32_t numVertices = 0;
    uint32_t numIndices  = 0;
    bool transparent = false;
    // We also store V/F for ray casting
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    // Per-vertex smooth normals (rows == V.rows(), 3 cols)
    Eigen::MatrixXf N;
};

class Renderer {
public:
    Renderer() = default;

    // Initialize bgfx with the given native window handle and dimensions.
    bool Init(void* nativeWindowHandle, uint16_t width, uint16_t height);
    void Shutdown();

    void BeginFrame(uint16_t width, uint16_t height);
    void EndFrame();

    // Upload a mesh to the GPU. The mesh vertices are face-expanded so each
    // triangle vertex carries the face color via abgr.
    // V: Nx3 vertices, F: Mx3 faces, abgr: per-face color (M entries)
    int UploadMesh(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F,
                   uint32_t faceColor);

    // Change face color for an already-uploaded mesh (re-uploads vertex buffer)
    void SetMeshColor(int meshId, uint32_t abgr);

    // Set whether a mesh should be drawn with alpha blending
    void SetMeshTransparent(int meshId, bool transparent);

    // Submit a mesh for drawing this frame
    void DrawMesh(int meshId);

    // Draw a line segment (for coordinate axes, etc.)
    void DrawLines(const std::vector<PosColorVertex>& vertices);

    // Destroy a specific mesh
    void DestroyMesh(int meshId);

    // Set the view and projection matrices for the current frame
    void SetViewTransform(const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj);

    // Set light direction, camera position, specular power, and global intensity for Phong shading
    void SetLightUniforms(const Eigen::Vector3f& lightDir, const Eigen::Vector3f& eyePos, float specularPower = 32.0f, float lightIntensity = 1.0f);
    // Enable or disable double-sided normals globally
    void SetDoubleSided(bool enabled);
    // Draw all meshes (opaque first, then transparent sorted back-to-front)
    void DrawAllMeshes(const Eigen::Vector3f& eyePos);

    // Access stored mesh data for ray casting
    const GpuMesh* GetMesh(int meshId) const;
    int GetMeshCount() const { return static_cast<int>(meshes_.size()); }

    // Pack an RGB float color (0-1 each) into ABGR uint32
    static uint32_t PackColor(float r, float g, float b, float a = 1.0f);

    // Handle window resize (must call bgfx::reset)
    void HandleResize(uint16_t width, uint16_t height);

    // Whether the current backend uses homogeneous depth [-1,1] vs [0,1]
    bool IsHomogeneousDepth() const;

    // View ID constants
    static constexpr bgfx::ViewId kMainView = 0;

private:
    bool LoadShaders();

    bgfx::ProgramHandle meshProgram_ = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle lineProgram_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_lightDir_  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_eyePos_    = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_lightIntensity_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_doubleSided_ = BGFX_INVALID_HANDLE;
    float lightDir_[4] = { 0.3f, 1.0f, 0.5f, 0.0f };
    float eyePos_[4]   = { 0.0f, 0.0f, 5.0f, 32.0f };
    float lightIntensity_[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float doubleSided_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    std::vector<GpuMesh> meshes_;
    bool initialized_ = false;
};

} // namespace Debugger3DS
