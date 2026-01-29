#include "render/Window.h"
#include <GLFW/glfw3.h>
#include <cstdint>

Window::Window(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
}

Window::~Window(){
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::shouldClose()const{
    return  glfwWindowShouldClose(m_window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

const char** Window::getExtensions(uint32_t &count) const {
    return glfwGetRequiredInstanceExtensions(&count);
}

GLFWwindow* Window::getWindow() const{
    return m_window;
}

void Window::getSize(int &width, int &height) const {
    glfwGetFramebufferSize(m_window, &width, &height);
}

void Window::setTitle(const char* title) {
    glfwSetWindowTitle(m_window, title);
}