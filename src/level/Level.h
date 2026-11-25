// =========================================================================
// Level data: visible mesh plus the .lvl sidecar
// =========================================================================
#ifndef __level__Level__h__
#define __level__Level__h__

#include <level/ObjLoader.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace level {
struct Brush {
  glm::vec3 min{0.0f}, max{0.0f};
  int trapDps = 0;
  // -1 means "not a door".
  int doorKey = -1;
  bool enabled = true;
};

struct KeyPickup {
  int id = 0;
  glm::vec3 pos{0.0f};
  bool taken = false;
};

struct EnemySpawn {
  // TODO: replace with an enum.
  // 0 = melee, 1 = ranged (shooter), 2 = tank (heavy).
  int type = 0;
  // The enemy patrols in a straight line from "spawn" to "pEnd".
  // Without a patrol instruction both points are equal.
  glm::vec3 spawn{0.0f}, pEnd{0.0f};
};

struct Light {
  glm::vec3 pos{0.0f};
  glm::vec3 color{1.0f};
  float intensity = 1.0f;
};

struct ExitZone {
  glm::vec3 min{0.0f}, max{0.0f};
  // Level file to load after touching this exit. Empty means "no next level".
  std::string nextLevel;
};

struct Level {
  // Path without extension. "<basePath>.obj" is the visible mesh and
  // "<basePath>.lvl" is the file passed to Load().
  std::string basePath;
  ObjMesh mesh;
  std::vector<Brush> brushes;
  std::vector<KeyPickup> keys;
  std::vector<EnemySpawn> enemies;
  std::vector<Light> lights;
  ExitZone exit;
  bool hasExit = false;
  glm::vec3 playerSpawn{0.0f};

  /**
   * Loads the OBJ mesh and the .lvl sidecar. The mesh path is derived from the
   * .lvl path by replacing its extension, so both files must share a base
   * name. Returns false and fills "error" (when provided) on the first
   * problem; the level is left untouched on failure.
   */
  bool Load(const std::string &lvlPath, std::string *error);
};

} // namespace level

#endif // __level__Level__h__

// eof - Level.h
