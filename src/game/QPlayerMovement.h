// =========================================================================
// Quake-style player movement
// =========================================================================
#ifndef __game__QPlayerMovement__h__
#define __game__QPlayerMovement__h__

#include <physics/Physics.h>

#include <glm/glm.hpp>

namespace physics {
class CollisionWorld;
} // namespace physics

namespace game {
// Per-step input snapshot produced by main.cpp.
struct Input {
  glm::vec3 WishDir{0.0f};
  bool JumpPressed = false;
  bool JumpHeld = false;
};

// Tuning values inspired by Quake 1, scaled to metres.
struct MovementConfig {
  float MaxSpeed = 8.0f;
  float GroundAccelerate = 10.0f;
  float AirAccelerate = 10.0f;
  float Friction = 6.0f;
  float StopSpeed = 2.5f;
  float JumpSpeed = 6.9f;
  float MaxFallSpeed = 50.0f;
  // Quake clamps the air-acceleration wish speed to a small value.
  float AirWishSpeedCap = 0.8f;
  // Held jump re-jumps on landing; otherwise only a fresh press jumps.
  bool AutoBhop = false;
};

class QPlayerMovement {
public:
  MovementConfig Config;

  /**
   * Applies friction/acceleration/jump/gravity and resolves the resulting
   * displacement against the static collision world.
   */
  void Step(physics::Body &body, const Input &input,
            physics::CollisionWorld &collision, float dt) const;

private:
  void _ApplyFriction(physics::Body &body, float dt) const;
  void _Accelerate(physics::Body &body, const glm::vec3 &wishDir,
                   float wishSpeed, float accelerate, float dt) const;
  void _AirAccelerate(physics::Body &body, const glm::vec3 &wishDir,
                      float wishSpeed, float accelerate, float dt) const;
};
} // namespace game

#endif // __game__QPlayerMovement__h__

// eof - QPlayerMovement.h
