// =========================================================================
// World state initialization and per-step update
// =========================================================================

#include <game/World.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>

namespace {
// -------------------------------------------------------------------------
game::EnemyStats _EnemyStatsFor(game::EnemyType type) {
  // moveSpeed, sightRange, attackRange, attackInterval, flashTime, maxHealth, damage
  switch (type) {
  case game::EnemyType::Melee:
    return {6.0f, 35.0f, 1.2f, 0.8f, 0.15f, 50, 10};
  case game::EnemyType::Ranged:
    return {4.5f, 45.0f, 15.0f, 1.5f, 0.20f, 30, 8};
  case game::EnemyType::Tank:
    return {1.8f, 35.0f, 20.0f, 2.0f, 0.25f, 120, 15};
  } // end switch

  return {3.0f, 35.0f, 1.2f, 0.8f, 0.15f, 50, 10};
}
} // namespace

// -------------------------------------------------------------------------
void game::World::Initialize(const level::Level &level) {
  this->state = GameState::Playing;
  this->player = physics::Body{};
  this->player.position =
      level.playerSpawn + glm::vec3(0.0f, this->player.halfExtents.y, 0.0f);
  this->player.velocity = glm::vec3(0.0f);
  this->player.onGround = false;
  this->playerHealth.maximum = 100;
  this->playerHealth.current = this->playerHealth.maximum;

  this->collision.Build(level.brushes);

  this->doorOpen.assign(level.brushes.size(), false);
  this->keyTaken.assign(level.keys.size(), false);
  this->keysHeld.clear();
  this->levelComplete = false;
  this->nextLevel.clear();

  this->weapon.cooldown = 0.0f;
  this->weapon.lastHit = physics::RayHit{};
  this->weapon.lastEnemyIndex = -1;
  this->trapDamageAccumulator = 0.0f;

  this->enemies.clear();
  for (const level::EnemySpawn &spawn : level.enemies) {
    Enemy enemy;
    this->_BuildEnemy(spawn, enemy);
    this->enemies.push_back(enemy);
  } // end for
}

// -------------------------------------------------------------------------
glm::vec3 game::World::PlayerFeet() const {
  return (this->player.position -
          glm::vec3(0.0f, this->player.halfExtents.y, 0.0f));
}

// -------------------------------------------------------------------------
void game::World::Update(float dt, const Input &input,
                         const level::Level &level) {
  if (this->state != GameState::Playing)
    return;

  if (!this->playerHealth.IsAlive()) {
    this->state = GameState::PlayerDead;
    return;
  } // end if

  // Pickups and doors run before movement so a door can open on the same step
  // the player reaches it.
  this->_CheckKeyPickups(level);
  this->_CheckDoors(level);

  this->movement.Step(this->player, input, this->collision, dt);
  this->_CheckTraps(level, dt);

  if (!this->playerHealth.IsAlive()) {
    this->state = GameState::PlayerDead;
    return;
  } // end if

  for (Enemy &enemy : this->enemies) {
    const int damage = enemy.Update(this->player, this->collision, dt);
    if (damage > 0) {
      const int applied = this->playerHealth.ApplyDamage(damage);
      std::cout << "enemy hit: " << applied
                << " player hp=" << this->playerHealth.current << std::endl;
    } // end if

    if (!this->playerHealth.IsAlive()) {
      this->state = GameState::PlayerDead;
      return;
    } // end if
  } // end for

  this->_UpdateWeapon(input, dt);

  this->_CheckExit(level);
}

// -------------------------------------------------------------------------
bool game::World::_PlayerTouches(const glm::vec3 &min, const glm::vec3 &max,
                                 float margin) const {
  const physics::Aabb body = physics::BodyAabb(this->player);
  const physics::Aabb box = physics::Expanded(physics::Aabb{min, max}, margin);
  return (physics::Overlaps(body, box));
}

// -------------------------------------------------------------------------
bool game::World::_HasKey(int keyId) const {
  for (int held : this->keysHeld) {
    if (held == keyId)
      return (true);
  } // end for
  return (false);
}

// -------------------------------------------------------------------------
void game::World::_CheckKeyPickups(const level::Level &level) {
  constexpr float kPickupHalfSize = 0.2f;
  const glm::vec3 half(kPickupHalfSize);

  for (std::size_t i = 0; i < level.keys.size(); ++i) {
    if (this->keyTaken[i])
      continue;

    const level::KeyPickup &key = level.keys[i];
    if (!this->_PlayerTouches(key.pos - half, key.pos + half, 0.0f))
      continue;

    this->keyTaken[i] = true;
    this->keysHeld.push_back(key.id);
  } // end for
}

// -------------------------------------------------------------------------
void game::World::_CheckDoors(const level::Level &level) {
  // The door is solid, so the player never overlaps it exactly. A small margin
  // turns "standing against the door" into a touch.
  constexpr float kDoorTouchMargin = 0.08f;

  for (std::size_t i = 0; i < level.brushes.size(); ++i) {
    const level::Brush &brush = level.brushes[i];
    if (brush.doorKey < 0 || this->doorOpen[i])
      continue;
    if (!this->_HasKey(brush.doorKey))
      continue;
    if (!this->_PlayerTouches(brush.min, brush.max, kDoorTouchMargin))
      continue;

    this->doorOpen[i] = true;
    this->collision.SetEnabled(static_cast<int>(i), false);
  } // end for
}

// -------------------------------------------------------------------------
void game::World::_CheckTraps(const level::Level &level, float dt) {
  constexpr float kTrapTouchMargin = 0.08f;

  int totalDps = 0;
  for (std::size_t i = 0; i < level.brushes.size(); ++i) {
    const level::Brush &brush = level.brushes[i];
    if (brush.trapDps <= 0)
      continue;
    if (!this->_PlayerTouches(brush.min, brush.max, kTrapTouchMargin))
      continue;
    totalDps += brush.trapDps;
  } // end for

  if (totalDps <= 0) {
    this->trapDamageAccumulator = 0.0f;
    return;
  } // end if

  this->trapDamageAccumulator += static_cast<float>(totalDps) * dt;
  const int damage = static_cast<int>(this->trapDamageAccumulator);
  if (damage <= 0)
    return;

  this->trapDamageAccumulator -= static_cast<float>(damage);
  const int applied = this->playerHealth.ApplyDamage(damage);
  if (applied > 0) {
    std::cout << "trap hit: " << applied
              << " player hp=" << this->playerHealth.current << std::endl;
  } // end if
}

// -------------------------------------------------------------------------
void game::World::_CheckExit(const level::Level &level) {
  if (this->levelComplete || level.exits.empty())
    return;

  for (const level::ExitZone &exit : level.exits) {
    if (!this->_PlayerTouches(exit.min, exit.max, 0.0f))
      continue;

    this->levelComplete = true;
    if (!exit.nextLevel.empty())
      this->nextLevel = exit.nextLevel;
    return;
  } // end for
}

// -------------------------------------------------------------------------
void game::World::_UpdateWeapon(const Input &input, float dt) {
  this->weapon.cooldown =
      std::max(0.0f, this->weapon.cooldown - dt);

  if (!input.firePressed || this->weapon.cooldown > 0.0f)
    return;

  const float directionLengthSq =
      glm::dot(input.aimDirection, input.aimDirection);
  if (directionLengthSq < 1e-8f)
    return;

  physics::Ray ray;
  ray.origin = input.aimOrigin;
  ray.direction = input.aimDirection / std::sqrt(directionLengthSq);
  ray.maxDistance = this->weapon.range;

  this->weapon.cooldown = this->weapon.fireInterval;

  const physics::RayHit wallHit = this->collision.Raycast(ray);
  physics::RayHit enemyHit;
  int enemyIndex = -1;

  for (std::size_t i = 0; i < this->enemies.size(); ++i) {
    Enemy &enemy = this->enemies[i];
    if (!enemy.health.IsAlive())
      continue;

    physics::RayHit hit;
    if (!this->collision.RaycastAabb(ray, physics::BodyAabb(enemy.body), hit))
      continue;

    if (enemyIndex < 0 || hit.distance < enemyHit.distance) {
      hit.brushIndex = -1;
      enemyHit = hit;
      enemyIndex = static_cast<int>(i);
    } // end if
  } // end for

  const bool enemyIsCloser =
      enemyIndex >= 0 &&
      (!wallHit.hit || enemyHit.distance < wallHit.distance);

  this->weapon.lastEnemyIndex = enemyIsCloser ? enemyIndex : -1;
  this->weapon.lastHit = enemyIsCloser ? enemyHit : wallHit;

  if (enemyIsCloser) {
    Enemy &enemy = this->enemies[static_cast<std::size_t>(enemyIndex)];
    const int applied = enemy.health.ApplyDamage(this->weapon.damage);
    if (applied > 0)
      enemy.hitFlashTimer = std::max(enemy.hitFlashTimer, 0.1f);

    std::cout << "hitscan: enemy " << enemyIndex << " damage " << applied
              << " hp=" << enemy.health.current << std::endl;
    return;
  } // end if

  if (wallHit.hit) {
    std::cout << "hitscan: brush " << wallHit.brushIndex << " at "
              << wallHit.distance << " m" << std::endl;
  } else {
    std::cout << "hitscan: miss" << std::endl;
  } // end if
}


// -------------------------------------------------------------------------
void game::World::_BuildEnemy(const level::EnemySpawn &enemyData,
                              Enemy &enemy) {
  enemy = Enemy{};

  const int typeIndex = (enemyData.type >= 0 && enemyData.type <= 2)
                            ? enemyData.type
                            : 0;
  enemy.type = static_cast<EnemyType>(typeIndex);
  enemy.stats = _EnemyStatsFor(enemy.type);

  if (enemy.type == EnemyType::Melee)
    enemy.body.halfExtents = glm::vec3(0.3f);
  else if (enemy.type == EnemyType::Tank)
    enemy.body.halfExtents = glm::vec3(0.7f, 1.1f, 0.7f);

  enemy.body.position = enemyData.spawn +
                        glm::vec3(0.0f, enemy.body.halfExtents.y, 0.0f);
  enemy.health.maximum = enemy.stats.maxHealth;
  enemy.health.current = enemy.health.maximum;
  enemy.patrolStart = enemy.body.position;
  enemy.patrolEnd = enemyData.pEnd +
                    glm::vec3(0.0f, enemy.body.halfExtents.y, 0.0f);
  enemy.patrolTarget = enemy.patrolEnd;
  enemy.movingToEnd = true;
  enemy.state = EnemyState::Patrol;
}

// eof - World.cxx
