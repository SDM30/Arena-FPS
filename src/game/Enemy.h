// =========================================================================
// Enemy runtime state and finite-state behavior
// =========================================================================
#ifndef __game__Enemy__h__
#define __game__Enemy__h__

#include <game/Health.h>
#include <physics/Physics.h>

#include <glm/glm.hpp>

namespace physics {
class CollisionWorld;
} // namespace physics

namespace game {
enum class EnemyType { Melee, Ranged, Tank };
enum class EnemyState { Patrol, Chase, Attack };

struct EnemyStats {
  float moveSpeed = 0.0f;
  float sightRange = 0.0f;
  float attackRange = 0.0f;
  float attackInterval = 0.0f;
  float flashTime = 0.0f;
  int maxHealth = 0;
  int damage = 0;
};

struct Enemy {
  EnemyType type = EnemyType::Melee;
  EnemyStats stats;
  physics::Body body;
  Health health;

  glm::vec3 patrolStart{0.0f};
  glm::vec3 patrolEnd{0.0f};
  glm::vec3 patrolTarget{0.0f};
  bool movingToEnd = true;

  EnemyState state = EnemyState::Patrol;
  float attackCooldown = 0.0f;
  // Attack/shot indicator (magenta).
  float flashTimer = 0.0f;
  // Damage-taken indicator (red).
  float hitFlashTimer = 0.0f;

  /**
   * Runs one fixed step of patrol/chase/attack behavior. Returns the damage
   * dealt to the player on this step (zero if no attack landed).
   */
  int Update(const physics::Body &player,
             physics::CollisionWorld &collision, float dt);

private:
  bool _CanSee(const physics::Body &player,
               const physics::CollisionWorld &collision) const;
  float _PlanarDistance(const glm::vec3 &target) const;
  void _Patrol(physics::CollisionWorld &collision, float dt);
  void _MoveToward(const glm::vec3 &target, float speed,
                   physics::CollisionWorld &collision, float dt);
};
} // namespace game

#endif // __game__Enemy__h__

// eof - Enemy.h
