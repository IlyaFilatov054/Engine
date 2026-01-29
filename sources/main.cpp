#include "render/Window.h"
#include "render/VkRenderer.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <string>

int main() {
    Window window;
    VkRenderer renderer(&window);

    auto start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;

    while (!window.shouldClose()) {
        window.pollEvents();
        renderer.render();

        frameCount++;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() > 1000){
            window.setTitle(std::to_string(frameCount).data());
            frameCount = 0;
            start = std::chrono::high_resolution_clock::now();
        }
    }

    return 0;
}