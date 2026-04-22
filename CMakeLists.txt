# Require a modern version of CMake
cmake_minimum_required(VERSION 3.15)

# Name your project
project(VIE-Project)

# Tell CMake we are using modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- NEW: Fetch GLFW automatically ---
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
# Turn off extra GLFW stuff we don't need to speed up compile time
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
# ---------------------------------------

# Tell CMake where to look for our .h header files
include_directories(include)

# Add BOTH of our .cpp files to the executable
add_executable(VIE-Project 
    src/main.cpp 
    src/Engine/Window.cpp
)

# Tell CMake to link the GLFW library to our executable
target_link_libraries(VIE-Project PRIVATE glfw)