# Require a modern version of CMake
cmake_minimum_required(VERSION 3.15)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
project(VIE-Project)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

# --- GLFW (Window & Input) ---
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- Bullet Physics (Collisions & Gravity) ---
FetchContent_Declare(bullet GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git GIT_TAG 3.25)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(bullet)

# --- NEW: Find the built-in OpenGL library on your computer ---
find_package(OpenGL REQUIRED)

include_directories(include)

add_executable(VIE-Project src/main.cpp src/Engine/Window.cpp)

# --- NEW: Link OpenGL::GL to our project ---
target_link_libraries(VIE-Project PRIVATE glfw BulletDynamics BulletCollision LinearMath OpenGL::GL)