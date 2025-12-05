#include "render/Window.h"

int main() {
    Window window;
    while (!window.shouldClose()) {
        window.pollEvents();
    }

    return 0;
}