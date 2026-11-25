// =========================================================================
// World state initialization and per-step update
// =========================================================================

#include <game/World.h>

// -------------------------------------------------------------------------
void game::World::Initialize(const level::Level &level) {
  this->player = physics::Body{};
  this->player.Position =
      level.playerSpawn + glm::vec3(0.0f, this->player.HalfExtents.y, 0.0f);
  this->player.Velocity = glm::vec3(0.0f);
  this->player.OnGround = false;

  this->collision.Build(level.brushes);

  this->doorOpen.assign(level.brushes.size(), false);
  this->keyTaken.assign(level.keys.size(), false);
  this->keysHeld.clear();
}

// -------------------------------------------------------------------------
glm::vec3 game::World::PlayerFeet() const {
  return (this->player.Position -
          glm::vec3(0.0f, this->player.HalfExtents.y, 0.0f));
}

// -------------------------------------------------------------------------
void game::World::Update(float dt) {
  const physics::MoveResult result = this->collision.MoveBody(this->player, dt);
  this->player.OnGround = result.HitGround;
}

// eof - World.cxx
