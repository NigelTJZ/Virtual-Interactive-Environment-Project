#pragma once
#include <string>

// Forward declaration so we don't have to include all of GLFW in the header
struct GLFWwindow; 

class Window {
public:
    // Constructor: Called when the window is created
    Window(int width, int height, const std::string& title);
    
    // Destructor: Called when the window is destroyed
    ~Window();

    // ---> ADD THIS LINE RIGHT HERE <---
    // This allows main.cpp to give ImGui the window pointer it needs
    GLFWwindow* getRawWindow() { return m_window; }

    // Core loop functions
    bool shouldClose();
    void pollEvents();
    void swapBuffers();
    
    bool isKeyPressed(int key);
    float getTime();
private:
    GLFWwindow* m_window; // A pointer to the actual GLFW window
};