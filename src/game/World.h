// =========================================================================
// Stores world state
// =========================================================================
#ifndef __game__World__h__
#define __game__World__h__

#include <level/Level.h>

#include <glm/glm.hpp>

#include <vector>

namespace game {
struct World {
  // Feet position of the player. The camera is derived from it by adding the
  // eye height, so physics works with the ground point and the renderer with
  // the eye.
  glm::vec3 playerPosition{0.0f};

  // Runtime flags, one entry per level definition. The level itself stays
  // immutable; restarting it only requires calling Initialize again.
  std::vector<bool> doorOpen;
  std::vector<bool> keyTaken;
  std::vector<int> keysHeld;

  /**
   * Copies the level's initial state into the runtime. Call it after
   * Level::Load and again to restart the level; it never reads files.
   */
  void Initialize(const level::Level &level);
};
} // namespace game

#endif // __game__World__h__

// eof - World.h
