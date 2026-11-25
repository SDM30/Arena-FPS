// =========================================================================
// Stores world state
// =========================================================================
#ifndef __game__World__h__
#define __game__World__h__

#include <level/Level.h>
#include <physics/CollisionWorld.h>
#include <physics/Physics.h>

#include <glm/glm.hpp>

#include <vector>

namespace game {
struct World {
  // Moving player body. Position is the center of its AABB.
  physics::Body player;

  // Static collision geometry copied from the level brushes.
  physics::CollisionWorld collision;

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

  /**
   * Feet position derived from the body center. The camera sits at eye height
   * above this point.
   */
  glm::vec3 PlayerFeet() const;

  /**
   * Advances the player by one fixed step. Input and game rules will be added
   * in the movement milestone.
   */
  void Update(float dt);
};
} // namespace game

#endif // __game__World__h__

// eof - World.h
