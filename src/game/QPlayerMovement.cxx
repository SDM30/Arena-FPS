// =========================================================================
// Quake-style player movement
// =========================================================================

#include <game/QPlayerMovement.h>

#include <physics/CollisionWorld.h>

#include <algorithm>
#include <cmath>

// -------------------------------------------------------------------------
void game::QPlayerMovement::Step(physics::Body &body, const Input &input,
                                 physics::CollisionWorld &collision,
                                 float dt) const {
  if (this->Config.FreeMove) {
    this->_FreeMove(body, input, dt);
    return;
  } // end if

  const bool wantsJump =
      this->Config.AutoBhop ? input.JumpHeld : input.JumpPressed;

  if (body.OnGround && wantsJump) {
    // Skip ground friction on the jump step so bunny hops keep their speed.
    body.Velocity.y = this->Config.JumpSpeed;
    body.OnGround = false;
  } else if (body.OnGround) {
    this->_ApplyFriction(body, dt);
    this->_Accelerate(body, input.WishDir, this->Config.MaxSpeed,
                      this->Config.GroundAccelerate, dt);
  } else {
    this->_AirAccelerate(body, input.WishDir, this->Config.MaxSpeed,
                         this->Config.AirAccelerate, dt);
  } // end if

  body.Velocity.y += physics::kGravity * dt;
  if (body.Velocity.y < -this->Config.MaxFallSpeed)
    body.Velocity.y = -this->Config.MaxFallSpeed;

  const glm::vec3 delta = body.Velocity * dt;
  const physics::MoveResult result = collision.Move(body, delta);
  body.OnGround = result.HitGround;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_FreeMove(physics::Body &body, const Input &input,
                                      float dt) const {
  glm::vec3 direction = input.WishDir;
  direction.y = (input.MoveUp ? 1.0f : 0.0f) -
                (input.MoveDown ? 1.0f : 0.0f);

  const float lengthSq = glm::dot(direction, direction);
  if (lengthSq > 1e-8f)
    direction /= std::sqrt(lengthSq);
  else
    direction = glm::vec3(0.0f);

  body.Velocity = direction * this->Config.FreeMoveSpeed;
  body.Position += body.Velocity * dt;
  body.OnGround = false;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_ApplyFriction(physics::Body &body,
                                           float dt) const {
  const float speed =
      std::sqrt((body.Velocity.x * body.Velocity.x) +
                (body.Velocity.z * body.Velocity.z));

  if (speed < 0.1f) {
    body.Velocity.x = 0.0f;
    body.Velocity.z = 0.0f;
    return;
  } // end if

  const float control = (speed < this->Config.StopSpeed)
                            ? this->Config.StopSpeed
                            : speed;
  const float drop = control * this->Config.Friction * dt;
  const float newSpeed = std::max(0.0f, speed - drop);
  const float scale = newSpeed / speed;

  body.Velocity.x *= scale;
  body.Velocity.z *= scale;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_Accelerate(physics::Body &body,
                                        const glm::vec3 &wishDir,
                                        float wishSpeed, float accelerate,
                                        float dt) const {
  const float currentSpeed = glm::dot(body.Velocity, wishDir);
  const float addSpeed = wishSpeed - currentSpeed;
  if (addSpeed <= 0.0f)
    return;

  float accelSpeed = accelerate * wishSpeed * dt;
  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  body.Velocity += wishDir * accelSpeed;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_AirAccelerate(physics::Body &body,
                                           const glm::vec3 &wishDir,
                                           float wishSpeed, float accelerate,
                                           float dt) const {
  // clamps the speed it is willing to add along the wish direction.
  const float clampedWishSpeed =
      std::min(wishSpeed, this->Config.AirWishSpeedCap);
  const float currentSpeed = glm::dot(body.Velocity, wishDir);
  const float addSpeed = clampedWishSpeed - currentSpeed;
  if (addSpeed <= 0.0f)
    return;

  float accelSpeed = accelerate * wishSpeed * dt;
  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  body.Velocity += wishDir * accelSpeed;
}

// eof - QPlayerMovement.cxx
