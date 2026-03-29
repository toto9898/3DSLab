#include "imgui_impl_bgfx.h"
#include <imgui.h>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <algorithm>
#include <cstring>

// We build a minimal shader for ImGui: position (2D) + texcoord + color
// Instead of embedding compiled shaders, we create the program at runtime
// from bgfx's built-in vertex/fragment shaders for 2D textured rendering.

// Simple vertex layout for ImGui
static bgfx::VertexLayout  s_imguiLayout;
static bgfx::TextureHandle s_fontTexture = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle s_texUniform  = BGFX_INVALID_HANDLE;
static bgfx::ProgramHandle s_program     = BGFX_INVALID_HANDLE;
static bgfx::ViewId        s_viewId      = 255;

// Embedded shaders for ImGui (position2d + texcoord + color)
// We'll create these from the same embedded shader mechanism as the mesh shaders.
// However, for ImGui we need a different vertex format, so we compile separate shaders.

// For simplicity, we'll use bgfx's transient buffers and a manually created shader.
// The shader source is compiled at build time alongside the mesh shaders.
#include "spirv/vs_imgui.sc.bin.h"
#include "spirv/fs_imgui.sc.bin.h"
#if BX_PLATFORM_WINDOWS
#include "dxbc/vs_imgui.sc.bin.h"
#include "dxbc/fs_imgui.sc.bin.h"
#include "dxil/vs_imgui.sc.bin.h"
#include "dxil/fs_imgui.sc.bin.h"
#endif
#include "glsl/vs_imgui.sc.bin.h"
#include "glsl/fs_imgui.sc.bin.h"
#include "essl/vs_imgui.sc.bin.h"
#include "essl/fs_imgui.sc.bin.h"

static const bgfx::EmbeddedShader s_imguiEmbeddedShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_imgui),
    BGFX_EMBEDDED_SHADER(fs_imgui),
    BGFX_EMBEDDED_SHADER_END()
};

bool ImGui_ImplBgfx_Init(bgfx::ViewId viewId) {
    s_viewId = viewId;

    // Setup vertex layout
    s_imguiLayout.begin()
        .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
        .end();

    // Create shader program
    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(s_imguiEmbeddedShaders, type, "vs_imgui");
    bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(s_imguiEmbeddedShaders, type, "fs_imgui");
    s_program = bgfx::createProgram(vsh, fsh, true);

    s_texUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

    // Build font atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    s_fontTexture = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false, 1,
        bgfx::TextureFormat::RGBA8,
        0,
        bgfx::copy(pixels, width * height * 4));

    io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(s_fontTexture.idx)));

    return true;
}

void ImGui_ImplBgfx_Shutdown() {
    if (bgfx::isValid(s_fontTexture)) bgfx::destroy(s_fontTexture);
    if (bgfx::isValid(s_texUniform)) bgfx::destroy(s_texUniform);
    if (bgfx::isValid(s_program)) bgfx::destroy(s_program);
    s_fontTexture = BGFX_INVALID_HANDLE;
    s_texUniform  = BGFX_INVALID_HANDLE;
    s_program     = BGFX_INVALID_HANDLE;
}

void ImGui_ImplBgfx_NewFrame() {
    // Nothing needed per-frame
}

void ImGui_ImplBgfx_RenderDrawLists(ImDrawData* drawData) {
    if (!drawData || drawData->TotalVtxCount == 0)
        return;

    const ImGuiIO& io = ImGui::GetIO();
    const float width  = io.DisplaySize.x;
    const float height = io.DisplaySize.y;

    // Setup orthographic projection
    {
        float ortho[16];
        // Column-major 4x4 orthographic matrix
        std::memset(ortho, 0, sizeof(ortho));
        ortho[0]  =  2.0f / width;
        ortho[5]  = -2.0f / height;   // flip Y
        ortho[10] = -1.0f;
        ortho[12] = -1.0f;
        ortho[13] =  1.0f;
        ortho[14] =  0.0f;
        ortho[15] =  1.0f;

        bgfx::setViewMode(s_viewId, bgfx::ViewMode::Sequential);
        bgfx::setViewRect(s_viewId, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
        bgfx::setViewTransform(s_viewId, nullptr, ortho);
    }

    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* drawList = drawData->CmdLists[n];

        // Create transient vertex buffer
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer  tib;

        uint32_t numVerts   = static_cast<uint32_t>(drawList->VtxBuffer.Size);
        uint32_t numIndices = static_cast<uint32_t>(drawList->IdxBuffer.Size);

        if (bgfx::getAvailTransientVertexBuffer(numVerts, s_imguiLayout) < numVerts) continue;
        if (bgfx::getAvailTransientIndexBuffer(numIndices) < numIndices) continue;
        bgfx::allocTransientVertexBuffer(&tvb, numVerts, s_imguiLayout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        // Copy vertex data
        auto* verts = reinterpret_cast<ImDrawVert*>(tvb.data);
        std::memcpy(verts, drawList->VtxBuffer.Data, numVerts * sizeof(ImDrawVert));

        // Copy index data
        auto* indices = reinterpret_cast<ImDrawIdx*>(tib.data);
        std::memcpy(indices, drawList->IdxBuffer.Data, numIndices * sizeof(ImDrawIdx));

        uint32_t idxOffset = 0;
        for (int cmd_i = 0; cmd_i < drawList->CmdBuffer.Size; ++cmd_i) {
            const ImDrawCmd& cmd = drawList->CmdBuffer[cmd_i];

            if (cmd.UserCallback) {
                cmd.UserCallback(drawList, &cmd);
                continue;
            }

            if (cmd.ElemCount == 0) continue;

            // Set scissor
            uint16_t sx = static_cast<uint16_t>(std::max(cmd.ClipRect.x, 0.0f));
            uint16_t sy = static_cast<uint16_t>(std::max(cmd.ClipRect.y, 0.0f));
            uint16_t sw = static_cast<uint16_t>(std::min(cmd.ClipRect.z, width)  - sx);
            uint16_t sh = static_cast<uint16_t>(std::min(cmd.ClipRect.w, height) - sy);
            bgfx::setScissor(sx, sy, sw, sh);

            // Set texture
            bgfx::TextureHandle texHandle;
            texHandle.idx = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(cmd.GetTexID()));
            bgfx::setTexture(0, s_texUniform, texHandle);

            bgfx::setVertexBuffer(0, &tvb, 0, numVerts);
            bgfx::setIndexBuffer(&tib, idxOffset, cmd.ElemCount);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                          | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                          | BGFX_STATE_MSAA);

            bgfx::submit(s_viewId, s_program);

            idxOffset += cmd.ElemCount;
        }
    }
}
