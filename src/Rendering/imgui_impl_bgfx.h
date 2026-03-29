#pragma once

#include <bgfx/bgfx.h>

struct ImDrawData;

// Minimal Dear ImGui rendering backend for bgfx.
// Uses a dedicated bgfx view for ImGui rendering.

bool    ImGui_ImplBgfx_Init(bgfx::ViewId viewId);
void    ImGui_ImplBgfx_Shutdown();
void    ImGui_ImplBgfx_NewFrame();
void    ImGui_ImplBgfx_RenderDrawLists(ImDrawData* drawData);
