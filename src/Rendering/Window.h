#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <cstdint>

namespace Debugger3DS::Rendering {

/// @brief Per-frame snapshot of mouse, scroll, and keyboard input.
struct InputState {
    double mouseX = 0; ///< Current mouse X position in window coordinates.
    double mouseY = 0; ///< Current mouse Y position in window coordinates.
    double scrollDeltaY = 0;     ///< Accumulated scroll delta this frame; reset each frame.
    bool mouseButtons[3] = {};   ///< Current button state: [left, right, middle].
    bool mouseJustPressed[3] = {};  ///< @c true on the first frame the button was pressed.
    bool mouseJustReleased[3] = {}; ///< @c true on the first frame the button was released.
    int lastKey = -1;      ///< Last key pressed this frame (GLFW key code, or -1).
    int lastKeyMod = 0;    ///< Modifier flags for @c lastKey.
    bool keyConsumed = false; ///< Set to @c true by UI code to suppress further key processing.
};

/// @brief GLFW window wrapper that owns the input state and native handle.
class Window {
public:
    Window() = default;
    ~Window();

    /// @brief Create and show a window.
    /// @return @c true on success.
    bool Init(int width, int height, const std::string& title);
    /// @brief Destroy the window and release GLFW resources.
    void Shutdown();
    /// @brief Update the window title bar text.
    void SetTitle(const std::string& title);

    bool ShouldClose() const; ///< @c true if the user has requested the window to close.
    /// @brief Process pending GLFW events.
    void PollEvents();

    /// @brief Return the platform-native window handle for bgfx initialization.
    void* GetNativeHandle() const;

    uint16_t GetWidth() const { return width_; }   ///< Logical window width in pixels.
    uint16_t GetHeight() const { return height_; } ///< Logical window height in pixels.
    int GetFramebufferWidth() const;  ///< Framebuffer width (may differ from logical width on HiDPI).
    int GetFramebufferHeight() const; ///< Framebuffer height (may differ from logical height on HiDPI).

    const InputState& GetInput() const { return input_; } ///< Read-only input state for this frame.
    InputState& GetInput() { return input_; }              ///< Mutable input state for this frame.

    /// @brief Clear per-frame transient input (called at start of each frame).
    void ResetFrameInput();

    GLFWwindow* GetGLFWWindow() const { return window_; } ///< Underlying GLFWwindow pointer.

    std::function<void(int, int)>          onResize; ///< Called when the framebuffer is resized.
    std::function<void(const std::string&)> onDrop;  ///< Called when a file is dropped onto the window.

private:
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* w, double x, double y);
    static void ScrollCallback(GLFWwindow* w, double xoff, double yoff);
    static void FramebufferResizeCallback(GLFWwindow* w, int width, int height);
    static void DropCallback(GLFWwindow* w, int count, const char** paths);

    GLFWwindow* window_ = nullptr;
    uint16_t width_ = 0, height_ = 0;
    InputState input_;
};

} // namespace Debugger3DS::Rendering::Rendering::Rendering
