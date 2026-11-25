// =========================================================================
// Enemy patrol/chase/attack finite-state behavior
// =========================================================================

#include <game/Enemy.h>

#include <physics/CollisionWorld.h>

#include <algorithm>
#include <cmath>

namespace {
constexpr float kArriveEpsilon = 0.1f;
constexpr float kEyeHeightFactor = 0.7f;
} // namespace

// -------------------------------------------------------------------------
int game::Enemy::Update(const physics::Body &player,
                        physics::CollisionWorld &collision, float dt) {
  if (!this->health.IsAlive())
    return (0);

  this->attackCooldown = std::max(0.0f, this->attackCooldown - dt);
  this->flashTimer = std::max(0.0f, this->flashTimer - dt);
  this->hitFlashTimer = std::max(0.0f, this->hitFlashTimer - dt);

  const bool seesPlayer = this->_CanSee(player, collision);
  const float distance = this->_PlanarDistance(player.position);
  int damage = 0;

  switch (this->state) {
  case EnemyState::Patrol:
    if (seesPlayer) {
      this->state = EnemyState::Chase;
      break;
    } // end if
    this->_Patrol(collision, dt);
    break;

  case EnemyState::Chase:
    if (!seesPlayer) {
      this->state = EnemyState::Patrol;
      break;
    } // end if
    if (distance <= this->stats.attackRange) {
      this->state = EnemyState::Attack;
      break;
    } // end if
    this->_MoveToward(player.position, this->stats.moveSpeed, collision, dt);
    break;

  case EnemyState::Attack:
    if (!seesPlayer) {
      this->state = EnemyState::Patrol;
      break;
    } // end if
    if (distance > this->stats.attackRange * 1.1f) {
      this->state = EnemyState::Chase;
      break;
    } // end if

    if (this->attackCooldown <= 0.0f) {
      this->attackCooldown = this->stats.attackInterval;
      this->flashTimer = this->stats.flashTime;
      damage = this->stats.damage;
    } // end if
    break;
  } // end switch

  return (damage);
}

// -------------------------------------------------------------------------
bool game::Enemy::_CanSee(const physics::Body &player,
                          const physics::CollisionWorld &collision) const {
  const glm::vec3 eye =
      this->body.position +
      glm::vec3(0.0f, this->body.halfExtents.y * kEyeHeightFactor, 0.0f);
  const glm::vec3 target =
      player.position +
      glm::vec3(0.0f, player.halfExtents.y * kEyeHeightFactor, 0.0f);

  const glm::vec3 toTarget = target - eye;
  const float distance = glm::length(toTarget);
  if (distance > this->stats.sightRange || distance < 1e-4f)
    return (false);

  physics::Ray ray;
  ray.origin = eye;
  ray.direction = toTarget / distance;
  ray.maxDistance = distance;

  const physics::RayHit hit = collision.Raycast(ray);
  return (!hit.hit || hit.distance >= distance - 0.05f);
}

// -------------------------------------------------------------------------
float game::Enemy::_PlanarDistance(const glm::vec3 &target) const {
  const glm::vec3 delta = target - this->body.position;
  return (std::sqrt((delta.x * delta.x) + (delta.z * delta.z)));
}

// -------------------------------------------------------------------------
void game::Enemy::_Patrol(physics::CollisionWorld &collision, float dt) {
  if (this->_PlanarDistance(this->patrolTarget) <= kArriveEpsilon) {
    this->movingToEnd = !this->movingToEnd;
    this->patrolTarget =
        this->movingToEnd ? this->patrolEnd : this->patrolStart;
  } // end if

  this->_MoveToward(this->patrolTarget, this->stats.moveSpeed, collision, dt);
}

// -------------------------------------------------------------------------
void game::Enemy::_MoveToward(const glm::vec3 &target, float speed,
                              physics::CollisionWorld &collision, float dt) {
  glm::vec3 delta = target - this->body.position;
  delta.y = 0.0f;

  const float length = glm::length(delta);
  if (length < 1e-4f)
    return;

  const glm::vec3 direction = delta / length;
  this->body.velocity = direction * speed;
  collision.Move(this->body, direction * speed * dt);
}

// eof - Enemy.cxx
