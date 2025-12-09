#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>

class Window {
public:
    Window();
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    const char** getExtensions(uint32_t &count) const;
    GLFWwindow* getWindow() const;
    void getSize(int &width, int &height) const;
private:
    GLFWwindow* m_window = nullptr;
};