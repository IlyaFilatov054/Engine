#pragma once

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
private:
    GLFWwindow* m_window = nullptr;
};