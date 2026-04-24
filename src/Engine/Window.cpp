#include "Engine/Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

Window::Window(int width, int height, const std::string& title) {
    // 1. Initialize the GLFW library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    // 2. Create the window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
    }
    
    // This connects OpenGL to this specific window so it knows where to draw!
    glfwMakeContextCurrent(m_window);
}

Window::~Window() {
    // Clean up when the game closes
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() {
    // Asks GLFW if the user clicked the 'X' button
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    // Tells GLFW to process keyboard and mouse inputs
    glfwPollEvents();
}

void Window::swapBuffers() {
    // Swaps the front and back drawing buffers (essential for smooth graphics later)
    glfwSwapBuffers(m_window);
}

bool Window::isKeyPressed(int key) {
    // Checks if a specific key is currently being held down
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

float Window::getTime() {
    // Returns the number of seconds since GLFW was initialized
    return (float)glfwGetTime();
}