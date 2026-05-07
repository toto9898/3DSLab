#include "Renderer.h"
#include "Mesh.h"
#include <bgfx/platform.h>
#include <bgfx/embedded_shader.h>
#include <bx/math.h>
#include <cstring>
#include <cmath>
#include <algorithm>

// Include embedded shader data (generated at build time)
#include "spirv/vs_mesh.sc.bin.h"
#include "spirv/fs_mesh.sc.bin.h"
#include "spirv/vs_line.sc.bin.h"
#include "spirv/fs_line.sc.bin.h"
#if BX_PLATFORM_WINDOWS
#include "dxbc/vs_mesh.sc.bin.h"
#include "dxbc/fs_mesh.sc.bin.h"
#include "dxbc/vs_line.sc.bin.h"
#include "dxbc/fs_line.sc.bin.h"
#include "dxil/vs_mesh.sc.bin.h"
#include "dxil/fs_mesh.sc.bin.h"
#include "dxil/vs_line.sc.bin.h"
#include "dxil/fs_line.sc.bin.h"
#endif
#include "glsl/vs_mesh.sc.bin.h"
#include "glsl/fs_mesh.sc.bin.h"
#include "glsl/vs_line.sc.bin.h"
#include "glsl/fs_line.sc.bin.h"
#include "essl/vs_mesh.sc.bin.h"
#include "essl/fs_mesh.sc.bin.h"
#include "essl/vs_line.sc.bin.h"
#include "essl/fs_line.sc.bin.h"

namespace Debugger3DS::Rendering {

using namespace Debugger3DS::Scene;

bgfx::VertexLayout PosColorVertex::layout;
bgfx::VertexLayout PosNormalColorVertex::layout;

static const bgfx::EmbeddedShader s_meshShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_mesh),
    BGFX_EMBEDDED_SHADER(fs_mesh),
    BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::EmbeddedShader s_lineShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_line),
    BGFX_EMBEDDED_SHADER(fs_line),
    BGFX_EMBEDDED_SHADER_END()
};

bool Renderer::Init(void* nativeWindowHandle, uint16_t width, uint16_t height) {
    bgfx::Init init;
    init.type = bgfx::RendererType::Count; // auto-detect
    init.platformData.nwh = nativeWindowHandle;
    init.resolution.width  = width;
    init.resolution.height = height;
    init.resolution.reset  = BGFX_RESET_VSYNC;

    if (!bgfx::init(init))
        return false;

    bgfx::setViewClear(kMainView,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff, 1.0f, 0);
    bgfx::setViewRect(kMainView, 0, 0, width, height);
    
    PosColorVertex::Init();
    PosNormalColorVertex::Init();

    SetDoubleSided(true);

    if (!LoadShaders())
        return false;

    initialized_ = true;
    return true;
}

void Renderer::Shutdown() {
    for (auto& mesh : meshes_) {
        if (bgfx::isValid(mesh.vbh)) bgfx::destroy(mesh.vbh);
        if (bgfx::isValid(mesh.ibh)) bgfx::destroy(mesh.ibh);
        if (bgfx::isValid(mesh.tex)) bgfx::destroy(mesh.tex);
    }
    meshes_.clear();

    if (bgfx::isValid(meshProgram_)) bgfx::destroy(meshProgram_);
    if (bgfx::isValid(lineProgram_)) bgfx::destroy(lineProgram_);
    if (bgfx::isValid(u_lightDir_)) bgfx::destroy(u_lightDir_);
    if (bgfx::isValid(u_eyePos_)) bgfx::destroy(u_eyePos_);
    if (bgfx::isValid(u_lightIntensity_)) bgfx::destroy(u_lightIntensity_);
    if (bgfx::isValid(u_doubleSided_)) bgfx::destroy(u_doubleSided_);
    if (bgfx::isValid(u_normalMatrix_)) bgfx::destroy(u_normalMatrix_);
    if (bgfx::isValid(s_texColor_)) bgfx::destroy(s_texColor_);
    if (bgfx::isValid(u_hasTexture_)) bgfx::destroy(u_hasTexture_);
    meshProgram_ = BGFX_INVALID_HANDLE;
    lineProgram_ = BGFX_INVALID_HANDLE;
    u_lightDir_  = BGFX_INVALID_HANDLE;
    u_eyePos_    = BGFX_INVALID_HANDLE;
    u_lightIntensity_ = BGFX_INVALID_HANDLE;
    u_doubleSided_ = BGFX_INVALID_HANDLE;
    u_normalMatrix_ = BGFX_INVALID_HANDLE;
    s_texColor_ = BGFX_INVALID_HANDLE;
    u_hasTexture_ = BGFX_INVALID_HANDLE;

    bgfx::shutdown();
    initialized_ = false;
}

bool Renderer::LoadShaders() {
    bgfx::RendererType::Enum type = bgfx::getRendererType();

    // Lit mesh program (position + normal + color)
    bgfx::ShaderHandle mvsh = bgfx::createEmbeddedShader(s_meshShaders, type, "vs_mesh");
    bgfx::ShaderHandle mfsh = bgfx::createEmbeddedShader(s_meshShaders, type, "fs_mesh");
    meshProgram_ = bgfx::createProgram(mvsh, mfsh, true);
    if (!bgfx::isValid(meshProgram_))
        return false;

    // Unlit line program (position + color only)
    bgfx::ShaderHandle lvsh = bgfx::createEmbeddedShader(s_lineShaders, type, "vs_line");
    bgfx::ShaderHandle lfsh = bgfx::createEmbeddedShader(s_lineShaders, type, "fs_line");
    lineProgram_ = bgfx::createProgram(lvsh, lfsh, true);
    if (!bgfx::isValid(lineProgram_))
        return false;

    // Create uniforms for Phong shading
    u_lightDir_ = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
    u_eyePos_   = bgfx::createUniform("u_eyePos",   bgfx::UniformType::Vec4);
    u_lightIntensity_ = bgfx::createUniform("u_lightIntensity", bgfx::UniformType::Vec4);
    u_doubleSided_ = bgfx::createUniform("u_doubleSided", bgfx::UniformType::Vec4);
    u_normalMatrix_ = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat4);
    s_texColor_ = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    u_hasTexture_ = bgfx::createUniform("u_hasTexture", bgfx::UniformType::Vec4);

    return true;
}

void Renderer::BeginFrame(uint16_t width, uint16_t height) {
    bgfx::setViewRect(kMainView, 0, 0, width, height);
    bgfx::touch(kMainView);
}

void Renderer::EndFrame() {
    bgfx::frame();
}

int Renderer::UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                          const uint16_t* indices, int nIndices,
                          uint32_t faceColor,
                          const Eigen::Matrix4f& modelMat) {
    int nFaces = nIndices / 3;
    std::vector<FaceMaterial> mats(static_cast<size_t>(nFaces));
    float r = (faceColor & 0xFF) / 255.0f;
    float g = ((faceColor >> 8) & 0xFF) / 255.0f;
    float b = ((faceColor >> 16) & 0xFF) / 255.0f;
    for (auto& m : mats) m.diffuse = Eigen::Vector3f(r, g, b);
    return UploadMesh(verts, indices, nIndices, mats, {}, BGFX_INVALID_HANDLE, modelMat);
}

int Renderer::UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                          const uint16_t* indices, int nIndices,
                          const std::vector<FaceMaterial>& faceMaterials,
                          const Eigen::Matrix4f& modelMat) {
    return UploadMesh(verts, indices, nIndices, faceMaterials, {}, BGFX_INVALID_HANDLE, modelMat);
}

int Renderer::UploadMesh(const std::vector<Eigen::Vector3f>& verts,
                          const uint16_t* indices, int nIndices,
                          const std::vector<FaceMaterial>& faceMaterials,
                          const std::vector<Eigen::Vector2f>& texCoords,
                          bgfx::TextureHandle tex,
                          const Eigen::Matrix4f& modelMat) {
    int nVerts = static_cast<int>(verts.size());
    int nFaces = nIndices / 3;

    // Accumulate area-weighted face normals per vertex (local space)
    Eigen::MatrixXf accum = Eigen::MatrixXf::Zero(nVerts, 3);

    for (int f = 0; f < nFaces; ++f) {
        int i0 = indices[3 * f], i1 = indices[3 * f + 1], i2 = indices[3 * f + 2];
        const Eigen::Vector3f& v0 = verts[i0];
        const Eigen::Vector3f& v1 = verts[i1];
        const Eigen::Vector3f& v2 = verts[i2];

        Eigen::Vector3f faceNormal = (v1 - v0).cross(v2 - v0);
        accum.row(i0) += faceNormal.transpose();
        accum.row(i1) += faceNormal.transpose();
        accum.row(i2) += faceNormal.transpose();
    }

    // Build shared-vertex buffer with per-vertex smooth normals and per-face material
    std::vector<PosNormalColorVertex> gpuVerts(static_cast<size_t>(nVerts));
    std::vector<bool> matAssigned(static_cast<size_t>(nVerts), false);
    struct VertMat { uint32_t diffuseAbgr; uint32_t ambientAbgr; float specR, specG, specB, shininess; };
    std::vector<VertMat> vertMats(static_cast<size_t>(nVerts));
    {
        VertMat def;
        def.diffuseAbgr = PackColor(0.8f, 0.8f, 0.8f, 1.0f);
        def.ambientAbgr = PackColor(0.2f, 0.2f, 0.2f, 0.0f);
        def.specR = 0.0f; def.specG = 0.0f; def.specB = 0.0f; def.shininess = 0.0f;
        std::fill(vertMats.begin(), vertMats.end(), def);
    }
    for (int f = 0; f < nFaces; ++f) {
        const auto& fm = (f < static_cast<int>(faceMaterials.size())) ? faceMaterials[static_cast<size_t>(f)] : faceMaterials[0];
        float opacity = 1.0f - fm.transparency;
        uint32_t diffAbgr = PackColor(fm.diffuse.x(), fm.diffuse.y(), fm.diffuse.z(), opacity);
        uint32_t ambAbgr  = PackColor(fm.ambient.x(), fm.ambient.y(), fm.ambient.z(), fm.selfIllumination);
        for (int c = 0; c < 3; ++c) {
            int vi = indices[3 * f + c];
            if (!matAssigned[static_cast<size_t>(vi)]) {
                vertMats[static_cast<size_t>(vi)].diffuseAbgr = diffAbgr;
                vertMats[static_cast<size_t>(vi)].ambientAbgr = ambAbgr;
                vertMats[static_cast<size_t>(vi)].specR = fm.specular.x() * fm.specularStrength;
                vertMats[static_cast<size_t>(vi)].specG = fm.specular.y() * fm.specularStrength;
                vertMats[static_cast<size_t>(vi)].specB = fm.specular.z() * fm.specularStrength;
                vertMats[static_cast<size_t>(vi)].shininess = fm.shininess;
                matAssigned[static_cast<size_t>(vi)] = true;
            }
        }
    }
    GpuMesh mesh;
    mesh.numVertices = static_cast<uint32_t>(nVerts);
    mesh.numIndices  = static_cast<uint32_t>(nIndices);
    mesh.modelMatrix = modelMat;
    mesh.tex = tex;

    // Local-space bbox accumulators for the 8-corner transform below
    Eigen::Vector3f localMin = Eigen::Vector3f::Constant(std::numeric_limits<float>::infinity());
    Eigen::Vector3f localMax = Eigen::Vector3f::Constant(-std::numeric_limits<float>::infinity());
    Eigen::Vector3f localSum = Eigen::Vector3f::Zero();

    for (int i = 0; i < nVerts; ++i) {
        Eigen::Vector3f n = accum.row(i).transpose();
        float len = n.norm();
        if (len > 1e-8f) n /= len;
        else n = Eigen::Vector3f(0, 1, 0);

        auto& v = gpuVerts[static_cast<size_t>(i)];
        v.x = verts[i].x();
        v.y = verts[i].y();
        v.z = verts[i].z();
        v.nx = n.x();
        v.ny = n.y();
        v.nz = n.z();
        v.abgr = vertMats[static_cast<size_t>(i)].diffuseAbgr;
        v.abgr1 = vertMats[static_cast<size_t>(i)].ambientAbgr;
        v.specR = vertMats[static_cast<size_t>(i)].specR;
        v.specG = vertMats[static_cast<size_t>(i)].specG;
        v.specB = vertMats[static_cast<size_t>(i)].specB;
        v.shininess = vertMats[static_cast<size_t>(i)].shininess;
        if (i < static_cast<int>(texCoords.size())) {
            v.u = texCoords[static_cast<size_t>(i)].x();
            v.v = texCoords[static_cast<size_t>(i)].y();
        } else {
            v.u = -1e6f;
            v.v = -1e6f;
        }

        // Track local-space bbox and centroid (scalar additions only, no matrix multiply)
        const auto& p = verts[i];
        localMin = localMin.cwiseMin(p);
        localMax = localMax.cwiseMax(p);
        localSum += p;
    }

    // Pre-compute normal matrix (inverse-transpose of upper-left 3x3)
    {
        Eigen::Matrix3f nm = modelMat.block<3, 3>(0, 0).inverse().transpose();
        mesh.normalMat[0]  = nm(0,0); mesh.normalMat[1]  = nm(0,1); mesh.normalMat[2]  = nm(0,2); mesh.normalMat[3]  = 0.0f;
        mesh.normalMat[4]  = nm(1,0); mesh.normalMat[5]  = nm(1,1); mesh.normalMat[6]  = nm(1,2); mesh.normalMat[7]  = 0.0f;
        mesh.normalMat[8]  = nm(2,0); mesh.normalMat[9]  = nm(2,1); mesh.normalMat[10] = nm(2,2); mesh.normalMat[11] = 0.0f;
        mesh.normalMat[12] = 0.0f;    mesh.normalMat[13] = 0.0f;    mesh.normalMat[14] = 0.0f;    mesh.normalMat[15] = 0.0f;
    }

    // Compute world-space centroid and bbox via 8-corner transform
    if (nVerts > 0) {
        Eigen::Vector3f localCentroid = localSum / static_cast<float>(nVerts);
        Eigen::Vector4f wc = modelMat * Eigen::Vector4f(localCentroid.x(), localCentroid.y(), localCentroid.z(), 1.0f);
        mesh.centroid = wc.head<3>();

        Eigen::Vector3f bmin = Eigen::Vector3f::Constant(std::numeric_limits<float>::infinity());
        Eigen::Vector3f bmax = Eigen::Vector3f::Constant(-std::numeric_limits<float>::infinity());
        for (int c = 0; c < 8; ++c) {
            Eigen::Vector3f corner(
                (c & 1) ? localMax.x() : localMin.x(),
                (c & 2) ? localMax.y() : localMin.y(),
                (c & 4) ? localMax.z() : localMin.z());
            Eigen::Vector4f wp = modelMat * Eigen::Vector4f(corner.x(), corner.y(), corner.z(), 1.0f);
            Eigen::Vector3f p = wp.head<3>();
            bmin = bmin.cwiseMin(p);
            bmax = bmax.cwiseMax(p);
        }
        mesh.bboxMin = bmin;
        mesh.bboxMax = bmax;
    }

    mesh.originalVertices = gpuVerts;

    const bgfx::Memory* vbMem = bgfx::copy(gpuVerts.data(), static_cast<uint32_t>(nVerts * sizeof(PosNormalColorVertex)));
    mesh.vbh = bgfx::createVertexBuffer(vbMem, PosNormalColorVertex::layout);

    const bgfx::Memory* ibMem = bgfx::copy(indices, static_cast<uint32_t>(nIndices * sizeof(uint16_t)));
    mesh.ibh = bgfx::createIndexBuffer(ibMem);  // 16-bit index buffer (no BGFX_BUFFER_INDEX32)

    int id = static_cast<int>(meshes_.size());
    meshes_.push_back(std::move(mesh));
    return id;
}

void Renderer::RestoreMesh(int meshId) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    auto& mesh = meshes_[meshId];
    if (mesh.originalVertices.empty()) return;

    if (bgfx::isValid(mesh.vbh)) bgfx::destroy(mesh.vbh);
    const bgfx::Memory* vbMem = bgfx::copy(mesh.originalVertices.data(),
        static_cast<uint32_t>(mesh.originalVertices.size() * sizeof(PosNormalColorVertex)));
    mesh.vbh = bgfx::createVertexBuffer(vbMem, PosNormalColorVertex::layout);
}

void Renderer::SetMeshColor(int meshId, uint32_t abgr) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    auto& mesh = meshes_[meshId];
    int nVerts = static_cast<int>(mesh.originalVertices.size());
    if (nVerts == 0) return;
    std::vector<PosNormalColorVertex> vertices(static_cast<size_t>(nVerts));
    uint32_t ambientAbgr = PackColor(0.15f, 0.15f, 0.15f, 0.0f);
    for (int i = 0; i < nVerts; ++i) {
        auto& v = vertices[static_cast<size_t>(i)];
        const auto& orig = mesh.originalVertices[static_cast<size_t>(i)];
        v.x = orig.x;
        v.y = orig.y;
        v.z = orig.z;
        v.nx = orig.nx;
        v.ny = orig.ny;
        v.nz = orig.nz;
        v.abgr = abgr;
        v.abgr1 = ambientAbgr;
        v.specR = 0.0f;
        v.specG = 0.0f;
        v.specB = 0.0f;
        v.shininess = 0.0f;
        v.u = 0.0f;
        v.v = 0.0f;
    }

    if (bgfx::isValid(mesh.vbh)) bgfx::destroy(mesh.vbh);
    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(), static_cast<uint32_t>(nVerts * sizeof(PosNormalColorVertex)));
    mesh.vbh = bgfx::createVertexBuffer(vbMem, PosNormalColorVertex::layout);
}

void Renderer::SetMeshTransparent(int meshId, bool transparent) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    meshes_[meshId].transparent = transparent;
}

void Renderer::DrawMesh(int meshId) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    const auto& mesh = meshes_[meshId];
    if (!bgfx::isValid(mesh.vbh)) return;

    // Set per-mesh model transform
    bgfx::setTransform(mesh.modelMatrix.data());

    bgfx::setVertexBuffer(0, mesh.vbh);
    if (bgfx::isValid(mesh.ibh))
        bgfx::setIndexBuffer(mesh.ibh);
    // Base state: write color, depth test, MSAA
    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA;

    // Culling: when double-sided rendering is disabled, enable backface culling
    if (doubleSided_[0] <= 0.5f)
        state |= BGFX_STATE_CULL_CW;

    if (mesh.transparent) {
        // Transparent: enable blending, but do not write depth
        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    } else {
        // Opaque: write depth
        state |= BGFX_STATE_WRITE_Z;
    }

    bgfx::setState(state);
    bgfx::setUniform(u_lightDir_, lightDir_);
    bgfx::setUniform(u_eyePos_, eyePos_);
    bgfx::setUniform(u_lightIntensity_, lightIntensity_);
    bgfx::setUniform(u_doubleSided_, doubleSided_);

    // Use pre-computed normal matrix (inverse-transpose of upper-left 3x3)
    bgfx::setUniform(u_normalMatrix_, mesh.normalMat, 1);

    // Texture binding
    float hasTexture[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (bgfx::isValid(mesh.tex)) {
        hasTexture[0] = 1.0f;
        bgfx::setTexture(0, s_texColor_, mesh.tex);
    }
    bgfx::setUniform(u_hasTexture_, hasTexture);

    bgfx::submit(kMainView, meshProgram_);
}

void Renderer::DrawAllMeshes(const Eigen::Vector3f& eyePos) {
    // First pass: opaque
    for (int i = 0; i < static_cast<int>(meshes_.size()); ++i) {
        if (!meshes_[i].transparent)
            DrawMesh(i);
    }
    // Second pass: transparent meshes (unsorted — avoids glitches during rotation)
    for (int i = 0; i < static_cast<int>(meshes_.size()); ++i) {
        if (meshes_[i].transparent)
            DrawMesh(i);
    }

    // Draw pivot marker if requested (small RGB axes at pivotPoint_)
    if (showPivotMarker_) {
        // Scale marker with camera distance so it's visible at different zoom levels
        float dist = (pivotPoint_ - eyePos).norm();
        float size = pivotMarkerSize_;
        if (dist > 1e-6f) size = std::max(pivotMarkerSize_, dist * 0.02f);

        std::vector<PosColorVertex> verts;
        verts.reserve(6);
        PosColorVertex v;
        uint32_t red = PackColor(1.0f, 0.0f, 0.0f);
        uint32_t green = PackColor(0.0f, 1.0f, 0.0f);
        uint32_t blue = PackColor(0.0f, 0.0f, 1.0f);

        // X axis
        v.x = pivotPoint_.x() - size; v.y = pivotPoint_.y(); v.z = pivotPoint_.z(); v.abgr = red; verts.push_back(v);
        v.x = pivotPoint_.x() + size; v.y = pivotPoint_.y(); v.z = pivotPoint_.z(); v.abgr = red; verts.push_back(v);
        // Y axis
        v.x = pivotPoint_.x(); v.y = pivotPoint_.y() - size; v.z = pivotPoint_.z(); v.abgr = green; verts.push_back(v);
        v.x = pivotPoint_.x(); v.y = pivotPoint_.y() + size; v.z = pivotPoint_.z(); v.abgr = green; verts.push_back(v);
        // Z axis
        v.x = pivotPoint_.x(); v.y = pivotPoint_.y(); v.z = pivotPoint_.z() - size; v.abgr = blue; verts.push_back(v);
        v.x = pivotPoint_.x(); v.y = pivotPoint_.y(); v.z = pivotPoint_.z() + size; v.abgr = blue; verts.push_back(v);

        DrawLines(verts);
    }
}

void Renderer::DrawLines(const std::vector<PosColorVertex>& vertices) {
    if (vertices.empty() || vertices.size() % 2 != 0) return;

    uint32_t numVerts = static_cast<uint32_t>(vertices.size());
    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer tib;

    if (bgfx::getAvailTransientVertexBuffer(numVerts, PosColorVertex::layout) < numVerts) return;
    if (bgfx::getAvailTransientIndexBuffer(numVerts) < numVerts) return;
    bgfx::allocTransientVertexBuffer(&tvb, numVerts, PosColorVertex::layout);
    bgfx::allocTransientIndexBuffer(&tib, numVerts);

    std::memcpy(tvb.data, vertices.data(), numVerts * sizeof(PosColorVertex));

    auto* idx = reinterpret_cast<uint16_t*>(tib.data);
    for (uint32_t i = 0; i < numVerts; ++i)
        idx[i] = static_cast<uint16_t>(i);

    bgfx::setVertexBuffer(0, &tvb);
    bgfx::setIndexBuffer(&tib);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z
                  | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_PT_LINES
                  | BGFX_STATE_LINEAA);
    bgfx::submit(kMainView, lineProgram_);
}

void Renderer::DestroyMesh(int meshId) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    auto& mesh = meshes_[meshId];
    if (bgfx::isValid(mesh.vbh)) { bgfx::destroy(mesh.vbh); mesh.vbh = BGFX_INVALID_HANDLE; }
    if (bgfx::isValid(mesh.ibh)) { bgfx::destroy(mesh.ibh); mesh.ibh = BGFX_INVALID_HANDLE; }
}

void Renderer::ClearAllMeshes() {
    for (auto& mesh : meshes_) {
        if (bgfx::isValid(mesh.vbh)) bgfx::destroy(mesh.vbh);
        if (bgfx::isValid(mesh.ibh)) bgfx::destroy(mesh.ibh);
        if (bgfx::isValid(mesh.tex)) bgfx::destroy(mesh.tex);
    }
    meshes_.clear();
}

void Renderer::SetMeshSource(int meshId, std::shared_ptr<Mesh> source, bool inverted) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    meshes_[meshId].sourceMesh = std::move(source);
    meshes_[meshId].invertedWinding = inverted;
}

void Renderer::SetViewTransform(const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj) {
    // bgfx expects column-major (which Eigen stores natively)
    bgfx::setViewTransform(kMainView, view.data(), proj.data());
}

void Renderer::SetLightUniforms(const Eigen::Vector3f& lightDir, const Eigen::Vector3f& eyePos, float specularPower, float lightIntensity) {
    lightDir_[0] = lightDir.x(); lightDir_[1] = lightDir.y(); lightDir_[2] = lightDir.z(); lightDir_[3] = 0.0f;
    eyePos_[0] = eyePos.x(); eyePos_[1] = eyePos.y(); eyePos_[2] = eyePos.z(); eyePos_[3] = specularPower;
    lightIntensity_[0] = lightIntensity; lightIntensity_[1] = 0.0f; lightIntensity_[2] = 0.0f; lightIntensity_[3] = 0.0f;
}

void Renderer::SetDoubleSided(bool enabled)
{
    doubleSided_[0] = enabled ? 1.0f : 0.0f;
}

void Renderer::SetPivotPoint(const Eigen::Vector3f& pivot)
{
    pivotPoint_ = pivot;
}

void Renderer::SetShowPivotMarker(bool show)
{
    showPivotMarker_ = show;
}

void Renderer::SetPivotMarkerSize(float size)
{
    pivotMarkerSize_ = size;
}

const GpuMesh* Renderer::GetMesh(int meshId) const {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return nullptr;
    return &meshes_[meshId];
}

void Renderer::HandleResize(uint16_t width, uint16_t height) {
    if (!initialized_ || width == 0 || height == 0) return;
    bgfx::reset(width, height, BGFX_RESET_VSYNC);
    bgfx::setViewRect(kMainView, 0, 0, width, height);
}

bool Renderer::IsHomogeneousDepth() const {
    return bgfx::getCaps()->homogeneousDepth;
}

uint32_t Renderer::PackColor(float r, float g, float b, float a) {
    uint8_t rr = static_cast<uint8_t>(r * 255.0f);
    uint8_t gg = static_cast<uint8_t>(g * 255.0f);
    uint8_t bb = static_cast<uint8_t>(b * 255.0f);
    uint8_t aa = static_cast<uint8_t>(a * 255.0f);
    return (aa << 24) | (bb << 16) | (gg << 8) | rr;
}

} // namespace Debugger3DS::Rendering
