#include "Renderer.h"
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

namespace Debugger3DS {

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
    }
    meshes_.clear();

    if (bgfx::isValid(meshProgram_)) bgfx::destroy(meshProgram_);
    if (bgfx::isValid(lineProgram_)) bgfx::destroy(lineProgram_);
    if (bgfx::isValid(u_lightDir_)) bgfx::destroy(u_lightDir_);
    if (bgfx::isValid(u_eyePos_)) bgfx::destroy(u_eyePos_);
    if (bgfx::isValid(u_lightIntensity_)) bgfx::destroy(u_lightIntensity_);
    if (bgfx::isValid(u_doubleSided_)) bgfx::destroy(u_doubleSided_);
    meshProgram_ = BGFX_INVALID_HANDLE;
    lineProgram_ = BGFX_INVALID_HANDLE;
    u_lightDir_  = BGFX_INVALID_HANDLE;
    u_eyePos_    = BGFX_INVALID_HANDLE;
    u_lightIntensity_ = BGFX_INVALID_HANDLE;
    u_doubleSided_ = BGFX_INVALID_HANDLE;

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

    return true;
}

void Renderer::BeginFrame(uint16_t width, uint16_t height) {
    bgfx::setViewRect(kMainView, 0, 0, width, height);
    bgfx::touch(kMainView);
}

void Renderer::EndFrame() {
    bgfx::frame();
}

int Renderer::UploadMesh(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F,
                          uint32_t faceColor) {
    int nVerts = static_cast<int>(V.rows());
    int nFaces = static_cast<int>(F.rows());

    // Accumulate area-weighted face normals per vertex
    Eigen::MatrixXf accum = Eigen::MatrixXf::Zero(nVerts, 3);
    std::vector<uint32_t> indices(static_cast<size_t>(nFaces) * 3);

    for (int f = 0; f < nFaces; ++f) {
        int i0 = F(f, 0), i1 = F(f, 1), i2 = F(f, 2);
        Eigen::Vector3f v0(static_cast<float>(V(i0, 0)), static_cast<float>(V(i0, 1)), static_cast<float>(V(i0, 2)));
        Eigen::Vector3f v1(static_cast<float>(V(i1, 0)), static_cast<float>(V(i1, 1)), static_cast<float>(V(i1, 2)));
        Eigen::Vector3f v2(static_cast<float>(V(i2, 0)), static_cast<float>(V(i2, 1)), static_cast<float>(V(i2, 2)));

        Eigen::Vector3f faceNormal = (v1 - v0).cross(v2 - v0);
        // area-weighted accumulation (faceNormal magnitude proportional to area)
        accum(i0, 0) += faceNormal.x(); accum(i0, 1) += faceNormal.y(); accum(i0, 2) += faceNormal.z();
        accum(i1, 0) += faceNormal.x(); accum(i1, 1) += faceNormal.y(); accum(i1, 2) += faceNormal.z();
        accum(i2, 0) += faceNormal.x(); accum(i2, 1) += faceNormal.y(); accum(i2, 2) += faceNormal.z();

        indices[3 * f + 0] = static_cast<uint32_t>(i0);
        indices[3 * f + 1] = static_cast<uint32_t>(i1);
        indices[3 * f + 2] = static_cast<uint32_t>(i2);
    }

    // Build shared-vertex buffer with per-vertex smooth normals
    std::vector<PosNormalColorVertex> vertices(static_cast<size_t>(nVerts));
    for (int i = 0; i < nVerts; ++i) {
        Eigen::Vector3f n(accum(i, 0), accum(i, 1), accum(i, 2));
        float len = n.norm();
        if (len > 1e-8f) n /= len;
        else n = Eigen::Vector3f(0, 1, 0);

        vertices[static_cast<size_t>(i)].x = static_cast<float>(V(i, 0));
        vertices[static_cast<size_t>(i)].y = static_cast<float>(V(i, 1));
        vertices[static_cast<size_t>(i)].z = static_cast<float>(V(i, 2));
        vertices[static_cast<size_t>(i)].nx = n.x();
        vertices[static_cast<size_t>(i)].ny = n.y();
        vertices[static_cast<size_t>(i)].nz = n.z();
        vertices[static_cast<size_t>(i)].abgr = faceColor;
    }

    GpuMesh mesh;
    mesh.numVertices = static_cast<uint32_t>(nVerts);
    mesh.numIndices  = static_cast<uint32_t>(nFaces * 3);
    mesh.V = V;
    mesh.F = F;
    mesh.N = accum;

    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(), static_cast<uint32_t>(nVerts * sizeof(PosNormalColorVertex)));
    mesh.vbh = bgfx::createVertexBuffer(vbMem, PosNormalColorVertex::layout);

    const bgfx::Memory* ibMem = bgfx::copy(indices.data(), static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
    mesh.ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);

    int id = static_cast<int>(meshes_.size());
    meshes_.push_back(std::move(mesh));
    return id;
}

void Renderer::SetMeshColor(int meshId, uint32_t abgr) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    auto& mesh = meshes_[meshId];
    // Rebuild vertex buffer with new color using stored per-vertex normals
    int nVerts = static_cast<int>(mesh.V.rows());
    std::vector<PosNormalColorVertex> vertices(static_cast<size_t>(nVerts));
    for (int i = 0; i < nVerts; ++i) {
        vertices[static_cast<size_t>(i)].x = static_cast<float>(mesh.V(i, 0));
        vertices[static_cast<size_t>(i)].y = static_cast<float>(mesh.V(i, 1));
        vertices[static_cast<size_t>(i)].z = static_cast<float>(mesh.V(i, 2));
        if (mesh.N.size() == mesh.V.rows()) {
            vertices[static_cast<size_t>(i)].nx = mesh.N(i, 0);
            vertices[static_cast<size_t>(i)].ny = mesh.N(i, 1);
            vertices[static_cast<size_t>(i)].nz = mesh.N(i, 2);
        } else {
            vertices[static_cast<size_t>(i)].nx = 0.0f;
            vertices[static_cast<size_t>(i)].ny = 1.0f;
            vertices[static_cast<size_t>(i)].nz = 0.0f;
        }
        vertices[static_cast<size_t>(i)].abgr = abgr;
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
    bgfx::submit(kMainView, meshProgram_);
}

void Renderer::DrawAllMeshes(const Eigen::Vector3f& eyePos) {
    // First pass: opaque
    for (int i = 0; i < static_cast<int>(meshes_.size()); ++i) {
        if (!meshes_[i].transparent)
            DrawMesh(i);
    }

    // Second pass: transparent (sorted back-to-front)
    std::vector<std::pair<float,int>> transparent;
    transparent.reserve(meshes_.size());
    for (int i = 0; i < static_cast<int>(meshes_.size()); ++i) {
        if (!meshes_[i].transparent) continue;
        const auto& m = meshes_[i];
        if (m.V.rows() > 0) {
            Eigen::Vector3d centroid = m.V.colwise().mean();
            Eigen::Vector3f c = centroid.cast<float>();
            float d2 = (c - eyePos).squaredNorm();
            transparent.emplace_back(d2, i);
        } else {
            transparent.emplace_back(0.0f, i);
        }
    }
    std::sort(transparent.begin(), transparent.end(), [](const auto& a, const auto& b){ return a.first > b.first; });
    for (auto &p : transparent)
        DrawMesh(p.second);
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

} // namespace Debugger3DS
