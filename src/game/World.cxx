// =========================================================================
// World state initialization from level data
// =========================================================================

#include <game/World.h>

// -------------------------------------------------------------------------
void game::World::Initialize(const level::Level &level) {
  this->playerPosition = level.playerSpawn;
  this->doorOpen.assign(level.brushes.size(), false);
  this->keyTaken.assign(level.keys.size(), false);
  this->keysHeld.clear();
}

// eof - World.cxx
