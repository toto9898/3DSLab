#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <cstdint>

namespace Debugger3DS {

struct InputState {
    double mouseX = 0, mouseY = 0;
    double scrollDeltaY = 0;     // accumulated this frame, reset each frame
    bool mouseButtons[3] = {};   // left, right, middle
    bool mouseJustPressed[3] = {};
    bool mouseJustReleased[3] = {};
    // Keyboard
    int lastKey = -1;
    int lastKeyMod = 0;
    bool keyConsumed = false;
};

class Window {
public:
    Window() = default;
    ~Window();

    bool Init(int width, int height, const std::string& title);
    void Shutdown();

    bool ShouldClose() const;
    void PollEvents();

    // Get the native window handle for bgfx
    void* GetNativeHandle() const;

    // Dimensions
    uint16_t GetWidth() const { return width_; }
    uint16_t GetHeight() const { return height_; }
    int GetFramebufferWidth() const;
    int GetFramebufferHeight() const;

    // Input state (updated each frame)
    const InputState& GetInput() const { return input_; }
    InputState& GetInput() { return input_; }

    // Reset per-frame transient input
    void ResetFrameInput();

    GLFWwindow* GetGLFWWindow() const { return window_; }

    // Callback for resize events
    std::function<void(int, int)> onResize;

private:
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* w, double x, double y);
    static void ScrollCallback(GLFWwindow* w, double xoff, double yoff);
    static void FramebufferResizeCallback(GLFWwindow* w, int width, int height);

    GLFWwindow* window_ = nullptr;
    uint16_t width_ = 0, height_ = 0;
    InputState input_;
};

} // namespace Debugger3DS
