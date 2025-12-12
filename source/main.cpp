#include "render/Window.h"
#include "render/VkRenderer.h"
#include <GLFW/glfw3.h>

int main() {
    Window window;
    VkRenderer renderer(&window);
    while (!window.shouldClose()) {
        window.pollEvents();
        renderer.render();
    }

    return 0;
}