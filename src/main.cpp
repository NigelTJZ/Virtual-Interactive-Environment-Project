#include "Engine/Window.h"
#include <GLFW/glfw3.h> // Needed for key codes
#include <iostream>

int main() {
    std::cout << "Starting the Engine..." << std::endl;
    Window myWindow(900, 650, "VIE Enginer SUPERMARIO!!");

    // Initialize our timer before the loop starts
    float lastTime = myWindow.getTime();

    // THE GAME LOOP
    while (!myWindow.shouldClose()) {
        
        // 1. Calculate Delta Time (dt)
        float currentTime = myWindow.getTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // 2. Process Input
        myWindow.pollEvents();

        // Test our new input system!
        if (myWindow.isKeyPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "Escape pressed. Closing engine..." << std::endl;
            break; 
        }

        // 3. Update Logic (This is where ECS and Physics will go)
        // Example of how we will use dt later:
        // mario.position.x += mario.velocity * deltaTime;

        // 4. Render
        myWindow.swapBuffers();
    }

    std::cout << "Engine shut down successfully." << std::endl;
    return 0;
}