#include "Renderer.h"
#include <bgfx/platform.h>
#include <bgfx/embedded_shader.h>
#include <bx/math.h>
#include <cstring>
#include <cmath>

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
    meshProgram_ = BGFX_INVALID_HANDLE;
    lineProgram_ = BGFX_INVALID_HANDLE;
    u_lightDir_  = BGFX_INVALID_HANDLE;
    u_eyePos_    = BGFX_INVALID_HANDLE;
    u_lightIntensity_ = BGFX_INVALID_HANDLE;

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
    int numFaces = static_cast<int>(F.rows());
    int numVerts = numFaces * 3; // face-expanded

    std::vector<PosNormalColorVertex> vertices(numVerts);
    std::vector<uint32_t> indices(numVerts);

    for (int f = 0; f < numFaces; ++f) {
        int i0 = F(f, 0), i1 = F(f, 1), i2 = F(f, 2);
        Eigen::Vector3f v0(static_cast<float>(V(i0, 0)), static_cast<float>(V(i0, 1)), static_cast<float>(V(i0, 2)));
        Eigen::Vector3f v1(static_cast<float>(V(i1, 0)), static_cast<float>(V(i1, 1)), static_cast<float>(V(i1, 2)));
        Eigen::Vector3f v2(static_cast<float>(V(i2, 0)), static_cast<float>(V(i2, 1)), static_cast<float>(V(i2, 2)));

        Eigen::Vector3f normal = (v1 - v0).cross(v2 - v0);
        float len = normal.norm();
        if (len > 1e-8f) normal /= len;
        else normal = Eigen::Vector3f(0, 1, 0);

        for (int v = 0; v < 3; ++v) {
            int vi = F(f, v);
            int idx = f * 3 + v;
            vertices[idx].x = static_cast<float>(V(vi, 0));
            vertices[idx].y = static_cast<float>(V(vi, 1));
            vertices[idx].z = static_cast<float>(V(vi, 2));
            vertices[idx].nx = normal.x();
            vertices[idx].ny = normal.y();
            vertices[idx].nz = normal.z();
            vertices[idx].abgr = faceColor;
            indices[idx] = static_cast<uint32_t>(idx);
        }
    }

    GpuMesh mesh;
    mesh.numVertices = numVerts;
    mesh.numIndices  = numVerts;
    mesh.V = V;
    mesh.F = F;

    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(), numVerts * sizeof(PosNormalColorVertex));
    mesh.vbh = bgfx::createVertexBuffer(vbMem, PosNormalColorVertex::layout);

    const bgfx::Memory* ibMem = bgfx::copy(indices.data(), numVerts * sizeof(uint32_t));
    mesh.ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);

    int id = static_cast<int>(meshes_.size());
    meshes_.push_back(std::move(mesh));
    return id;
}

void Renderer::SetMeshColor(int meshId, uint32_t abgr) {
    if (meshId < 0 || meshId >= static_cast<int>(meshes_.size())) return;
    auto& mesh = meshes_[meshId];

    // Rebuild vertex buffer with new color (preserving normals)
    int numFaces = static_cast<int>(mesh.F.rows());
    int numVerts = numFaces * 3;

    std::vector<PosNormalColorVertex> vertices(numVerts);
    for (int f = 0; f < numFaces; ++f) {
        int i0 = mesh.F(f, 0), i1 = mesh.F(f, 1), i2 = mesh.F(f, 2);
        Eigen::Vector3f v0(static_cast<float>(mesh.V(i0, 0)), static_cast<float>(mesh.V(i0, 1)), static_cast<float>(mesh.V(i0, 2)));
        Eigen::Vector3f v1(static_cast<float>(mesh.V(i1, 0)), static_cast<float>(mesh.V(i1, 1)), static_cast<float>(mesh.V(i1, 2)));
        Eigen::Vector3f v2(static_cast<float>(mesh.V(i2, 0)), static_cast<float>(mesh.V(i2, 1)), static_cast<float>(mesh.V(i2, 2)));

        Eigen::Vector3f normal = (v1 - v0).cross(v2 - v0);
        float len = normal.norm();
        if (len > 1e-8f) normal /= len;
        else normal = Eigen::Vector3f(0, 1, 0);

        for (int v = 0; v < 3; ++v) {
            int vi = mesh.F(f, v);
            int idx = f * 3 + v;
            vertices[idx].x = static_cast<float>(mesh.V(vi, 0));
            vertices[idx].y = static_cast<float>(mesh.V(vi, 1));
            vertices[idx].z = static_cast<float>(mesh.V(vi, 2));
            vertices[idx].nx = normal.x();
            vertices[idx].ny = normal.y();
            vertices[idx].nz = normal.z();
            vertices[idx].abgr = abgr;
        }
    }

    if (bgfx::isValid(mesh.vbh)) bgfx::destroy(mesh.vbh);
    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(), numVerts * sizeof(PosNormalColorVertex));
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

    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z
                   | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW
                   | BGFX_STATE_MSAA;
    if (mesh.transparent)
        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

    bgfx::setState(state);
    bgfx::setUniform(u_lightDir_, lightDir_);
    bgfx::setUniform(u_eyePos_, eyePos_);
    bgfx::setUniform(u_lightIntensity_, lightIntensity_);
    bgfx::submit(kMainView, meshProgram_);
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
