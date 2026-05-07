#pragma once

#include <bgfx/bgfx.h>
#include <Eigen/Dense>
#include <vector>
#include <cstdint>
#include <memory>

namespace Debugger3DS::Scene { class Mesh; }

namespace Debugger3DS::Rendering {

using Debugger3DS::Scene::Mesh;

/// @brief Per-vertex data for line rendering (position + ABGR color, no normal).
struct PosColorVertex {
    float x;    ///< X position.
    float y;    ///< Y position.
    float z;    ///< Z position.
    uint32_t abgr; ///< Packed color in ABGR byte order.

    /// @brief Initialize the bgfx vertex layout for this struct.
    static void Init() {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }
    static bgfx::VertexLayout layout; ///< Shared vertex layout descriptor.
};

/// @brief Per-vertex data for mesh rendering (position + normal + ABGR color + material).
struct PosNormalColorVertex {
    float x;  ///< X position.
    float y;  ///< Y position.
    float z;  ///< Z position.
    float nx; ///< X normal component.
    float ny; ///< Y normal component.
    float nz; ///< Z normal component.
    uint32_t abgr;   ///< Diffuse color (RGB) + transparency (A) packed as ABGR in Color0.
    uint32_t abgr1;  ///< Ambient color (RGB) + self-illumination (A) packed as ABGR in Color1.
    float specR;     ///< Specular red component (TexCoord1.x).
    float specG;     ///< Specular green component (TexCoord1.y).
    float specB;     ///< Specular blue component (TexCoord1.z).
    float shininess; ///< Specular exponent (TexCoord1.w).
    float u; ///< Texture U coordinate (TexCoord0.x).
    float v; ///< Texture V coordinate (TexCoord0.y).

    /// @brief Initialize the bgfx vertex layout for this struct.
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
    static bgfx::VertexLayout layout; ///< Shared vertex layout descriptor.
};

/// @brief Per-face material properties used during mesh upload.
struct FaceMaterial {
    Eigen::Vector3f ambient  = Eigen::Vector3f(0.2f, 0.2f, 0.2f); ///< Ambient color (RGB, 0–1).
    Eigen::Vector3f diffuse  = Eigen::Vector3f(0.8f, 0.8f, 0.8f); ///< Diffuse color (RGB, 0–1).
    Eigen::Vector3f specular = Eigen::Vector3f(0.0f, 0.0f, 0.0f); ///< Specular color (RGB, 0–1).
    float shininess = 0.0f;        ///< Specular exponent.
    float transparency = 0.0f;     ///< Transparency (0 = opaque, 1 = fully transparent).
    float selfIllumination = 0.0f; ///< Self-illumination factor (0–1).
    float specularStrength = 0.0f; ///< Specular strength multiplier.
};

/// @brief GPU-resident mesh handle returned by Renderer::UploadMesh().
struct GpuMesh {
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE; ///< bgfx vertex buffer handle.
    bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE; ///< bgfx index buffer handle.
    bgfx::TextureHandle      tex = BGFX_INVALID_HANDLE; ///< Texture handle (may be invalid).
    uint32_t numVertices = 0; ///< Number of vertices in the buffer.
    uint32_t numIndices  = 0; ///< Number of indices in the buffer.
    bool transparent = false; ///< @c true if the mesh should be drawn with alpha blending.
    Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity(); ///< Model-to-world transform.
    float normalMat[16] = {}; ///< Inverse-transpose of the upper-left 3×3 of modelMatrix, packed as mat4.
    std::shared_ptr<Mesh> sourceMesh;  ///< Source parsed mesh for ray casting (avoids duplicating data).
    bool invertedWinding = false;      ///< @c true if face winding was flipped on upload.
    std::vector<PosNormalColorVertex> originalVertices; ///< Backup of vertex data for restoring after highlight.
    Eigen::Vector3f centroid = Eigen::Vector3f::Zero(); ///< World-space centroid (computed at upload).
    Eigen::Vector3f bboxMin  = Eigen::Vector3f::Zero(); ///< World-space AABB minimum corner.
    Eigen::Vector3f bboxMax  = Eigen::Vector3f::Zero(); ///< World-space AABB maximum corner.
};

/// @brief bgfx-backed renderer for 3D mesh and line drawing.
class Renderer {
public:
    Renderer() = default;

    /// @brief Initialize bgfx with the given native window handle and dimensions.
    /// @return @c true on success.
    bool Init(void* nativeWindowHandle, uint16_t width, uint16_t height);
    /// @brief Shut down bgfx and release all GPU resources.
    void Shutdown();

    /// @brief Begin a new render frame.
    void BeginFrame(uint16_t width, uint16_t height);
    /// @brief Submit the current frame to the GPU.
    void EndFrame();

    /// @brief Upload a textured mesh to the GPU.
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   const std::vector<FaceMaterial>& faceMaterials,
                   const std::vector<Eigen::Vector2f>& texCoords,
                   bgfx::TextureHandle tex,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    /// @brief Upload a mesh with a single solid face color.
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   uint32_t faceColor,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    /// @brief Upload a mesh with per-face materials but no texture.
    int UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                   const uint16_t* indices, int nIndices,
                   const std::vector<FaceMaterial>& faceMaterials,
                   const Eigen::Matrix4f& modelMat = Eigen::Matrix4f::Identity());

    /// @brief Change the face color for an already-uploaded mesh.
    void SetMeshColor(int meshId, uint32_t abgr);

    /// @brief Restore a mesh to its original vertex data.
    void RestoreMesh(int meshId);

    /// @brief Enable or disable alpha blending for a mesh.
    void SetMeshTransparent(int meshId, bool transparent);

    /// @brief Submit a mesh for drawing this frame.
    void DrawMesh(int meshId);

    /// @brief Draw a polyline from a list of colored vertices.
    void DrawLines(const std::vector<PosColorVertex>& vertices);

    /// @brief Destroy a specific uploaded mesh and free its GPU resources.
    void DestroyMesh(int meshId);

    /// @brief Destroy all uploaded meshes (keeps the renderer alive).
    void ClearAllMeshes();

    /// @brief Associate a source mesh for ray casting.
    void SetMeshSource(int meshId, std::shared_ptr<Mesh> source, bool inverted);

    /// @brief Set the view and projection matrices for the current frame.
    void SetViewTransform(const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj);

    /// @brief Set Phong shading uniforms.
    void SetLightUniforms(const Eigen::Vector3f& lightDir, const Eigen::Vector3f& eyePos, float specularPower = 32.0f, float lightIntensity = 1.0f);
    /// @brief Enable or disable double-sided normals globally.
    void SetDoubleSided(bool enabled);
    /// @brief Draw all meshes (opaque first, then transparent).
    void DrawAllMeshes(const Eigen::Vector3f& eyePos);

    /// @brief Set the debug pivot marker position (world space).
    void SetPivotPoint(const Eigen::Vector3f& pivot);
    /// @brief Show or hide the pivot marker.
    void SetShowPivotMarker(bool show);
    /// @brief Set the pivot marker cross-hair size.
    void SetPivotMarkerSize(float size);

    /// @brief Access stored mesh data; returns @c nullptr for invalid IDs.
    const GpuMesh* GetMesh(int meshId) const;
    int GetMeshCount() const { return static_cast<int>(meshes_.size()); } ///< Number of uploaded meshes.

    /// @brief Pack an RGB float color (0–1) into a packed ABGR @c uint32.
    static uint32_t PackColor(float r, float g, float b, float a = 1.0f);

    /// @brief Notify the renderer of a window resize.
    void HandleResize(uint16_t width, uint16_t height);

    /// @brief Returns @c true if the backend uses homogeneous depth ([-1,1]).
    bool IsHomogeneousDepth() const;

    static constexpr bgfx::ViewId kMainView = 0; ///< bgfx view ID for the main 3D pass.

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

} // namespace Debugger3DS::Rendering
