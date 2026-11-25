// =========================================================================
// Minimal Dear ImGui HUD: health, speed and crosshair
// =========================================================================
#ifndef __ui__Hud__h__
#define __ui__Hud__h__

#include <string>

namespace game {
struct World;
} // namespace game

struct GLFWwindow;

namespace ui {
class Hud {
public:
  Hud() = default;
  ~Hud() = default;

  Hud(const Hud &) = delete;
  Hud &operator=(const Hud &) = delete;
  Hud(Hud &&) = delete;
  Hud &operator=(Hud &&) = delete;

  /**
   * Creates the ImGui context and GLFW/OpenGL backends. The GL context must
   * be current. Returns false and fills "error" on failure.
   */
  bool Initialize(GLFWwindow *window, std::string *error);

  /**
   * Draws the HUD over the current frame. Call after the 3D scene and before
   * swapping buffers.
   */
  void Render(const game::World &world);

  void Shutdown();

private:
  bool initialized = false;
};
} // namespace ui

#endif // __ui__Hud__h__

// eof - Hud.h
