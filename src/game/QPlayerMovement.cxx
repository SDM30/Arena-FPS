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
  if (this->Config.freeMove) {
    this->_FreeMove(body, input, dt);
    return;
  } // end if

  const bool wantsJump =
      this->Config.autoBhop ? input.jumpHeld : input.jumpPressed;

  if (body.onGround && wantsJump) {
    // Skip ground friction on the jump step so bunny hops keep their speed.
    body.velocity.y = this->Config.jumpSpeed;
    body.onGround = false;
  } else if (body.onGround) {
    this->_ApplyFriction(body, dt);
    this->_Accelerate(body, input.wishDir, this->Config.maxSpeed,
                      this->Config.groundAccelerate, dt);
  } else {
    this->_AirAccelerate(body, input.wishDir, this->Config.maxSpeed,
                         this->Config.airAccelerate, dt);
  } // end if

  body.velocity.y += physics::kGravity * dt;
  if (body.velocity.y < -this->Config.maxFallSpeed)
    body.velocity.y = -this->Config.maxFallSpeed;

  const glm::vec3 delta = body.velocity * dt;
  const physics::MoveResult result = collision.Move(body, delta);
  body.onGround = result.hitGround;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_FreeMove(physics::Body &body, const Input &input,
                                      float dt) const {
  glm::vec3 direction = input.wishDir;
  direction.y = (input.moveUp ? 1.0f : 0.0f) -
                (input.moveDown ? 1.0f : 0.0f);

  const float lengthSq = glm::dot(direction, direction);
  if (lengthSq > 1e-8f)
    direction /= std::sqrt(lengthSq);
  else
    direction = glm::vec3(0.0f);

  body.velocity = direction * this->Config.freeMoveSpeed;
  body.position += body.velocity * dt;
  body.onGround = false;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_ApplyFriction(physics::Body &body,
                                           float dt) const {
  const float speed =
      std::sqrt((body.velocity.x * body.velocity.x) +
                (body.velocity.z * body.velocity.z));

  if (speed < 0.1f) {
    body.velocity.x = 0.0f;
    body.velocity.z = 0.0f;
    return;
  } // end if

  const float control = (speed < this->Config.stopSpeed)
                            ? this->Config.stopSpeed
                            : speed;
  const float drop = control * this->Config.friction * dt;
  const float newSpeed = std::max(0.0f, speed - drop);
  const float scale = newSpeed / speed;

  body.velocity.x *= scale;
  body.velocity.z *= scale;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_Accelerate(physics::Body &body,
                                        const glm::vec3 &wishDir,
                                        float wishSpeed, float accelerate,
                                        float dt) const {
  const float currentSpeed = glm::dot(body.velocity, wishDir);
  const float addSpeed = wishSpeed - currentSpeed;
  if (addSpeed <= 0.0f)
    return;

  float accelSpeed = accelerate * wishSpeed * dt;
  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  body.velocity += wishDir * accelSpeed;
}

// -------------------------------------------------------------------------
void game::QPlayerMovement::_AirAccelerate(physics::Body &body,
                                           const glm::vec3 &wishDir,
                                           float wishSpeed, float accelerate,
                                           float dt) const {
  // clamps the speed it is willing to add along the wish direction.
  const float clampedWishSpeed =
      std::min(wishSpeed, this->Config.airWishSpeedCap);
  const float currentSpeed = glm::dot(body.velocity, wishDir);
  const float addSpeed = clampedWishSpeed - currentSpeed;
  if (addSpeed <= 0.0f)
    return;

  float accelSpeed = accelerate * wishSpeed * dt;
  if (accelSpeed > addSpeed)
    accelSpeed = addSpeed;

  body.velocity += wishDir * accelSpeed;
}

// eof - QPlayerMovement.cxx
