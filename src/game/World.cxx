// =========================================================================
// World state initialization and per-step update
// =========================================================================

#include <game/World.h>

#include <cstddef>

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
  this->levelComplete = false;
  this->nextLevel.clear();
}

// -------------------------------------------------------------------------
glm::vec3 game::World::PlayerFeet() const {
  return (this->player.Position -
          glm::vec3(0.0f, this->player.HalfExtents.y, 0.0f));
}

// -------------------------------------------------------------------------
void game::World::Update(float dt, const Input &input,
                         const level::Level &level) {
  // Pickups and doors run before movement so a door can open on the same step
  // the player reaches it.
  this->_CheckKeyPickups(level);
  this->_CheckDoors(level);

  this->movement.Step(this->player, input, this->collision, dt);

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
    if (!this->_PlayerTouches(brush.Min, brush.Max, kDoorTouchMargin))
      continue;

    this->doorOpen[i] = true;
    this->collision.SetEnabled(static_cast<int>(i), false);
  } // end for
}

// -------------------------------------------------------------------------
void game::World::_CheckExit(const level::Level &level) {
  if (!level.hasExit || this->levelComplete)
    return;

  if (this->_PlayerTouches(level.exit.Min, level.exit.Max, 0.0f)) {
    this->levelComplete = true;
    if (!level.exit.NextLevel.empty())
      this->nextLevel = level.exit.NextLevel;
  } // end if
}

// eof - World.cxx
