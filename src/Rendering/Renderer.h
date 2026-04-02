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

// Per-vertex data for mesh rendering (position + normal + ABGR color + material)
struct PosNormalColorVertex {
    float x, y, z;
    float nx, ny, nz;
    uint32_t abgr;      // diffuse color (RGB) + transparency (A) in Color0
    uint32_t abgr1;     // ambient color (RGB) + self-illumination (A) in Color1
    float specR, specG, specB, shininess; // specular color + exponent in TexCoord1
    float u, v;         // texture UV coordinates in TexCoord0

    static void Init() {
        layout.begin()
            .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::Color1,    4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
    static bgfx::VertexLayout layout;
};

// Per-face material properties for upload
struct FaceMaterial {
    Eigen::Vector3f ambient  = Eigen::Vector3f(0.2f, 0.2f, 0.2f);
    Eigen::Vector3f diffuse  = Eigen::Vector3f(0.8f, 0.8f, 0.8f);
    Eigen::Vector3f specular = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    float shininess = 0.0f;
    float transparency = 0.0f;
    float selfIllumination = 0.0f;
    float specularStrength = 0.0f;
};

// Handle returned when a mesh is uploaded to the GPU
struct GpuMesh {
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle      tex = BGFX_INVALID_HANDLE;
    uint32_t numVertices = 0;
    uint32_t numIndices  = 0;
    bool transparent = false;
    // Per-mesh model matrix (set via bgfx::setTransform at draw time)
    Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
    // Pre-computed normal matrix (inverse-transpose of upper-left 3x3, packed as mat4)
    float normalMat[16] = {};
    // Local-space vertices and flat face indices for ray casting
    std::vector<Eigen::Vector3f> localVerts;
    std::vector<uint16_t>        localIndices;   // flat triplets
    // Per-vertex smooth normals (local space, rows == localVerts.size(), 3 cols)
    Eigen::MatrixXf N;
    // Original vertex data for restoring after selection highlight
    std::vector<PosNormalColorVertex> originalVertices;
    // Cached centroid and bbox in world space (computed at upload time)
    Eigen::Vector3f centroid = Eigen::Vector3f::Zero();
    Eigen::Vector3f bboxMin  = Eigen::Vector3f::Zero();
    Eigen::Vector3f bboxMax  = Eigen::Vector3f::Zero();
};

class Renderer {
public:
    Renderer() = default;

    // Initialize bgfx with the given native window handle and dimensions.
    bool Init(void* nativeWindowHandle, uint16_t width, uint16_t height);
    void Shutdown();

    void BeginFrame(uint16_t width, uint16_t height);
    void EndFrame();

    // Upload a mesh to the GPU from flat arrays (zero-copy path).
    // verts: local-space positions, indices: flat uint16 triplets,
    // faceMaterials: one per face, texCoords: per-vertex UVs (optional),
    // tex: texture handle, modelMat: model-to-world transform.
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   const std::vector<FaceMaterial>& faceMaterials,
                   const std::vector<Eigen::Vector2f>& texCoords,
                   bgfx::TextureHandle tex,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    // Convenience: upload with a single face color
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   uint32_t faceColor,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    // Convenience: upload with per-face material (no texture)
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   const std::vector<FaceMaterial>& faceMaterials,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    // Change face color for an already-uploaded mesh (re-uploads vertex buffer)
    void SetMeshColor(int meshId, uint32_t abgr);

    // Restore mesh to its original vertex data (after selection highlight)
    void RestoreMesh(int meshId);

    // Set whether a mesh should be drawn with alpha blending
    void SetMeshTransparent(int meshId, bool transparent);

    // Submit a mesh for drawing this frame
    void DrawMesh(int meshId);

    // Draw a line segment (for coordinate axes, etc.)
    void DrawLines(const std::vector<PosColorVertex>& vertices);

    // Destroy a specific mesh
    void DestroyMesh(int meshId);

    // Destroy all meshes (GPU resources only, keeps renderer alive)
    void ClearAllMeshes();

    // Set the view and projection matrices for the current frame
    void SetViewTransform(const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj);

    // Set light direction, camera position, specular power, and global intensity for Phong shading
    void SetLightUniforms(const Eigen::Vector3f& lightDir, const Eigen::Vector3f& eyePos, float specularPower = 32.0f, float lightIntensity = 1.0f);
    // Enable or disable double-sided normals globally
    void SetDoubleSided(bool enabled);
    // Draw all meshes (opaque first, then transparent)
    void DrawAllMeshes(const Eigen::Vector3f& eyePos);

    // Pivot marker (debug): set pivot world position and toggle marker rendering
    void SetPivotPoint(const Eigen::Vector3f& pivot);
    void SetShowPivotMarker(bool show);
    void SetPivotMarkerSize(float size);

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
    bgfx::UniformHandle u_normalMatrix_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle s_texColor_ = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_hasTexture_ = BGFX_INVALID_HANDLE;
    float lightDir_[4] = { 0.3f, 1.0f, 0.5f, 0.0f };
    float eyePos_[4]   = { 0.0f, 0.0f, 5.0f, 32.0f };
    float lightIntensity_[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float doubleSided_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    std::vector<GpuMesh> meshes_;
    bool initialized_ = false;
    // Pivot marker state
    Eigen::Vector3f pivotPoint_ = Eigen::Vector3f::Zero();
    bool showPivotMarker_ = false;
    float pivotMarkerSize_ = 0.05f;
};

} // namespace Debugger3DS
