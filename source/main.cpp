#include "render/Window.h"
#include "render/VkRenderer.h"

int main() {
    Window window;
    VkRenderer renderer(&window);
    while (!window.shouldClose()) {
        window.pollEvents();
    }

    return 0;
}