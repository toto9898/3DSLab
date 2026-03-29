#include "Window.h"

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace Debugger3DS {

Window::~Window() {
    Shutdown();
}

bool Window::Init(int width, int height, const std::string& title) {
    if (!glfwInit())
        return false;

    // No OpenGL context — bgfx manages the graphics API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return false;
    }

    width_ = static_cast<uint16_t>(width);
    height_ = static_cast<uint16_t>(height);

    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetMouseButtonCallback(window_, MouseButtonCallback);
    glfwSetCursorPosCallback(window_, CursorPosCallback);
    glfwSetScrollCallback(window_, ScrollCallback);
    glfwSetFramebufferSizeCallback(window_, FramebufferResizeCallback);

    return true;
}

void Window::Shutdown() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return window_ && glfwWindowShouldClose(window_);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void* Window::GetNativeHandle() const {
#if defined(_WIN32)
    return glfwGetWin32Window(window_);
#else
    return nullptr;
#endif
}

int Window::GetFramebufferWidth() const {
    int w, h;
    glfwGetFramebufferSize(window_, &w, &h);
    return w;
}

int Window::GetFramebufferHeight() const {
    int w, h;
    glfwGetFramebufferSize(window_, &w, &h);
    return h;
}

void Window::ResetFrameInput() {
    input_.scrollDeltaY = 0;
    input_.lastKey = -1;
    input_.keyConsumed = false;
    for (int i = 0; i < 3; ++i) {
        input_.mouseJustPressed[i] = false;
        input_.mouseJustReleased[i] = false;
    }
}

void Window::KeyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        self->input_.lastKey = key;
        self->input_.lastKeyMod = mods;
    }
}

void Window::MouseButtonCallback(GLFWwindow* w, int button, int action, int /*mods*/) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || button < 0 || button > 2) return;
    if (action == GLFW_PRESS) {
        self->input_.mouseButtons[button] = true;
        self->input_.mouseJustPressed[button] = true;
    } else if (action == GLFW_RELEASE) {
        self->input_.mouseButtons[button] = false;
        self->input_.mouseJustReleased[button] = true;
    }
}

void Window::CursorPosCallback(GLFWwindow* w, double x, double y) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->input_.mouseX = x;
    self->input_.mouseY = y;
}

void Window::ScrollCallback(GLFWwindow* w, double /*xoff*/, double yoff) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->input_.scrollDeltaY += yoff;
}

void Window::FramebufferResizeCallback(GLFWwindow* w, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->width_ = static_cast<uint16_t>(width);
    self->height_ = static_cast<uint16_t>(height);
    if (self->onResize)
        self->onResize(width, height);
}

} // namespace Debugger3DS
