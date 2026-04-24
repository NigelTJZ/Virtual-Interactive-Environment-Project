# VIE-Project
For LTU VIE module project Spring 2026

Project name: SuperEngine3D
Team members: Nigel & Joseph

## 1. Engine Vision
We are building an engine tailored for precise, frame-perfect 3D kinematic platforming. Instead of realistic rigid-body physics, the engine prioritizes deterministic, arcade-style movement mechanics (snappy jumps, momentum-based running) combined with highly efficient, data-oriented entity management.

## 2. Target Genre
The engine is optimized for 3D platformers and obstacle course environments that require high-precision character control and responsive collision detection against static geometry.

## 3. Core Scenario (Currently Implemented)
The first playable technical scenario is a 3D bounded arena where a player-controlled entity can run in 8 directions, jump with variable height (based on input duration), and collide with a static platform floor. The scene features multiple autonomous patrol enemies (Goombas) that the player can jump on to defeat, alongside a classic 10 HP damage system. The level concludes with a dynamic "Flagpole" capture sequence that calculates a score based on jump height.

## 4. Reusable Engine Features (Architecture)
* **Entity Component System (ECS):** Implemented using a highly optimized Struct of Arrays (SoA) architecture for fast contiguous memory access.
* **Custom Kinematic Character Controller:** Written entirely from scratch to handle custom gravity, acceleration, and AABB/Capsule collision detection without relying on heavy external physics SDKs.
* **Game State Machine:** A core loop manager that seamlessly transitions the engine between states (`COUNTDOWN`, `PLAYING`, `FLAG_ANIMATION`, `STAGE_COMPLETED`, `GAME_OVER`).
* **Hierarchical 3D Modeler:** A custom graphics sub-system that dynamically scales, translates, and rotates basic geometric primitives to assemble complex, multi-colored characters (like the "Fall Guys" style player) in real time.
* **Orthographic 2D HUD Layer:** An integrated 2D projection system that overlays UI elements (like dynamic health bars and retro 7-segment countdown timers) directly onto the 3D viewport.

*(Planned: Spatial partitioning system, e.g., Grid-based spatial hashing, to optimize collision checks as entity counts grow).*

## 5. Game-Specific Features
* **"Mario-style" Jump & Movement:** Implemented variable jump curves (short hops vs. high jumps) and momentum-based horizontal movement.
* **Autonomous AI Agents:** Implemented Goomba-style enemies featuring a dual-state AI (wandering in a mathematical circle around a spawn point, snapping to a direct pursuit vector when the player enters a 5-meter aggro radius).
* **Combat & I-Frames:** Implemented damage handling with a 1.5-second visual blinking I-frame system to prevent rapid health drain.

*(Planned: Interactive dynamic objects like collectable coins or breakable blocks).*

## 6. Non-Goals for Milestone 1
* Complex rendering features (PBR materials, advanced lighting, shadows).
* Advanced audio processing (3D spatial audio).
* Network multiplayer.

## 7. Data-Driven Elements (Pending Implementation)
* Initial level layout and geometry loaded from a JSON/text file.
* Configurable physics parameters (gravity multiplier, max run speed, jump impulse) loaded externally to allow for rapid tweaking without recompiling.
* Entity spawn coordinates and counts.

## 8. Chosen Libraries and Tools
* **Graphics API:** Raw OpenGL (for maximum control, cross-platform compatibility, and zero build system friction).
* **Window & Input:** GLFW (Used for application window creation, OpenGL context management, and hardware key-state polling).
* **Physics:** Custom C++ kinematic solver (No external library used, ensuring precise, arcade-style platforming feel).
* **Build System:** CMake (Modern C++17 configuration).

*(Planned additions: `miniaudio` for sound effects, `nlohmann/json` for parsing data-driven configuration files).*

## 9. Engineering Rules
* **License:** MIT License.
* **Code style:** C++ Style
* **Folder structure:** (draft plan)
  * `/src` (Implementation files)
  * `/include` (Header files)
  * `/assets` (Models, textures, config files)
  * `/third_party` (External libraries like glfw)