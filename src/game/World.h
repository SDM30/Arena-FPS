// =========================================================================
// Stores world state
// =========================================================================
#ifndef __game__World__h__
#define __game__World__h__

#include <game/QPlayerMovement.h>
#include <level/Level.h>
#include <physics/CollisionWorld.h>
#include <physics/Physics.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace game {
struct World {
  // Moving player body. Position is the center of its AABB.
  physics::Body player;

  // Static collision geometry copied from the level brushes.
  physics::CollisionWorld collision;

  // Quake-style movement rules and tuning (including AutoBhop).
  QPlayerMovement movement;

  // Runtime flags, one entry per level definition. The level itself stays
  // immutable; restarting it only requires calling Initialize again.
  std::vector<bool> doorOpen;
  std::vector<bool> keyTaken;
  std::vector<int> keysHeld;
  bool levelComplete = false;

  // Set when the exit names a next level. main.cpp consumes it, reloads the
  // level and calls Initialize again.
  std::string nextLevel;

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
   * Advances the player by one fixed step using the movement controller, then
   * checks pickups, doors and the exit against the static level data.
   */
  void Update(float dt, const Input &input, const level::Level &level);

private:
  bool _PlayerTouches(const glm::vec3 &min, const glm::vec3 &max,
                      float margin) const;
  bool _HasKey(int keyId) const;
  void _CheckKeyPickups(const level::Level &level);
  void _CheckDoors(const level::Level &level);
  void _CheckExit(const level::Level &level);
};
} // namespace game

#endif // __game__World__h__

// eof - World.h
