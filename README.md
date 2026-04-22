# VIE-Project
For LTU VIE module project Spring 2026

Template:

Project name: SuperEngine3D
Team members: Nigel & Joseph

1. Engine vision
We are building an engine tailored for precise, frame perfect 3D kinematic platforming. Instead of realistic rigid-body physics, the engine prioritizes deterministic, arcade-style movement mechanics (snappy jumps, momentum-based running) combined with efficient entity management.

2. Target genre/VIE
The engine is optimized for 3D platformers and obstacle course environments that require high precision character control and responsive collision detection against complex static geometry.

3. Core scenario
The first playable technical scenario is a 3D sandbox environment where a player controlled capsule can run, jump with variable height (based on input duration), and collide with static terrain. The scene will also feature multiple autonomous agents (e.g. simple patrol enemies) that the player can interact with (e.g. bouncing off their heads).

4. Reusable engine features

Custom Kinematic Character Controller (handling gravity, acceleration, and AABB/Capsule collision detection).

Entity Component System (ECS) or Scene Graph for dynamic, configurable object management.

Spatial partitioning system (e.g., Grid based spatial hashing) to optimize collision checks and ensure the < 16.67ms update loop.

5. Game-specific features

"Mario-style" variable jump curve and momentum-based horizontal movement.

Basic autonomous patrol AI for enemy entities.

Interactive dynamic objects (e.g. collectable coins or breakable blocks).

6. Non-goals for milestone 1

Complex rendering features (PBR materials, advanced lighting, shadows).

Advanced audio processing (3D spatial audio).

Network multiplayer.

7. Data-driven elements

Initial level layout and geometry loaded from a JSON/text file.

Configurable physics parameters (gravity multiplier, max run speed, jump impulse) loaded externally to allow for rapid tweaking without recompiling.

Entity spawn coordinates and counts.

8. Chosen libraries and tools
Either BGFX (lightweight for custom C++ engine), or Ogre3D for heavier framework.

3D: bgfx (Cross-platform, low-overhead rendering).

Physics: Custom kinematic solver (No external library, to ensure precise platforming feel).

Audio: miniaudio (Single-file header, very easy to integrate).

Build: CMake.

Other: nlohmann/json (for parsing data-driven configuration files), GLFW or SDL2 (for window creation and input handling).

9. Engineering rules

License: MIT License.

Code style: Google C++ Style Guide (Enforced via .clang-format).

Folder structure:

/src (Implementation files)

/include (Header files)

/assets (Models, textures, config files)

/third_party (External libraries like bgfx, glfw)
