// =========================================================================
// Level loading: OBJ mesh + .lvl sidecar parser
//
// .lvl grammar (one instruction per line, '#' starts a comment):
//   player x y z
//   brush  minX minY minZ maxX maxY maxZ
//   door   brushIndex keyId
//   key    keyId x y z
//   exit   minX minY minZ maxX maxY maxZ
//   trap   brushIndex damagePerSecond
//   light  x y z r g b intensity
//   enemy  type x y z [x2 y2 z2]
// =========================================================================

#include <level/Level.h>

#include <cstddef>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace {
// -------------------------------------------------------------------------
bool _Fail(std::string *error, const std::string &file, int line,
           const std::string &message) {
  if (error != nullptr) {
    *error = file;
    if (line > 0)
      *error += ":" + std::to_string(line);
    *error += ": " + message;
  }
  return (false);
}

// -------------------------------------------------------------------------
bool _ReadFloat(std::istringstream &in, float &value) {
  return (static_cast<bool>(in >> value));
}

bool _ReadInt(std::istringstream &in, int &value) {
  return (static_cast<bool>(in >> value));
}

bool _ReadVec3(std::istringstream &in, glm::vec3 &value) {
  return (static_cast<bool>(in >> value.x >> value.y >> value.z));
}

// True when nothing but whitespace is left on the line.
bool _AtEnd(std::istringstream &in) {
  in >> std::ws;
  return (in.eof());
}

// -------------------------------------------------------------------------
// Enemy type names map to the int stored in EnemySpawn::type.
bool _ReadEnemyType(std::istringstream &in, int &type) {
  std::string name;
  if (!(in >> name))
    return (false);

  if (name == "melee") {
    type = 0;
  } else if (name == "ranged" || name == "shooter") {
    type = 1;
  } else if (name == "tank" || name == "heavy") {
    type = 2;
  } else {
    return (false);
  } // end if

  return (true);
}

// -------------------------------------------------------------------------
// "levels/nivel1.lvl" -> "levels/nivel1"
std::string _WithoutExtension(const std::string &path) {
  const std::size_t slash = path.find_last_of("/\\");
  const std::size_t dot = path.find_last_of('.');
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
    return (path);
  return (path.substr(0, dot));
}

// -------------------------------------------------------------------------
// Brush references are resolved after the whole file is read, so a door or a
// trap may appear before the brush it points to.
struct _PendingDoor {
  int line;
  int brushIndex;
  int keyId;
};

struct _PendingTrap {
  int line;
  int brushIndex;
  int damagePerSecond;
};
} // namespace

// -------------------------------------------------------------------------
bool level::Level::Load(const std::string &lvlPath, std::string *error) {
  if (error != nullptr)
    error->clear();

  Level parsed;
  parsed.basePath = _WithoutExtension(lvlPath);

  const std::string objPath = parsed.basePath + ".obj";
  if (!LoadObj(objPath, parsed.mesh, error))
    return (false);
  if (parsed.mesh.IsEmpty())
    return (_Fail(error, objPath, 0, "the mesh has no triangles"));

  std::ifstream input(lvlPath);
  if (!input)
    return (_Fail(error, lvlPath, 0, "could not open the level file"));

  std::vector<_PendingDoor> doors;
  std::vector<_PendingTrap> traps;
  std::set<int> keyIds;
  bool hasPlayer = false;

  std::string raw;
  int lineNumber = 0;

  while (std::getline(input, raw)) {
    ++lineNumber;

    // Erase the comment, then the '\r' left by CRLF files: getline() only
    // splits on '\n', so a Windows file keeps the carriage return at the end.
    if (const std::size_t hash = raw.find('#'); hash != std::string::npos)
      raw.erase(hash);
    if (!raw.empty() && raw.back() == '\r')
      raw.pop_back();

    std::istringstream line(raw);
    std::string instruction;
    if (!(line >> instruction))
      continue; // blank line

    if (instruction == "player") {
      if (!_ReadVec3(line, parsed.playerSpawn) || !_AtEnd(line))
        return (
            _Fail(error, lvlPath, lineNumber, "player needs exactly x y z"));
      hasPlayer = true;

    } else if (instruction == "brush") {
      Brush brush;
      if (!_ReadVec3(line, brush.Min) || !_ReadVec3(line, brush.Max) ||
          !_AtEnd(line))
        return (_Fail(error, lvlPath, lineNumber,
                      "brush needs exactly minX minY minZ maxX maxY maxZ"));
      if (brush.Min.x > brush.Max.x || brush.Min.y > brush.Max.y ||
          brush.Min.z > brush.Max.z)
        return (
            _Fail(error, lvlPath, lineNumber, "brush min is greater than max"));
      parsed.brushes.push_back(brush);

    } else if (instruction == "door") {
      _PendingDoor door{lineNumber, -1, -1};
      if (!_ReadInt(line, door.brushIndex) || !_ReadInt(line, door.keyId) ||
          !_AtEnd(line))
        return (_Fail(error, lvlPath, lineNumber,
                      "door needs exactly brushIndex keyId"));
      doors.push_back(door);

    } else if (instruction == "key") {
      KeyPickup key;
      if (!_ReadInt(line, key.id) || !_ReadVec3(line, key.pos) || !_AtEnd(line))
        return (
            _Fail(error, lvlPath, lineNumber, "key needs exactly keyId x y z"));
      if (key.id < 0)
        return (_Fail(error, lvlPath, lineNumber,
                      "key id must be zero or greater"));
      if (!keyIds.insert(key.id).second)
        return (_Fail(error, lvlPath, lineNumber,
                      "duplicate key id " + std::to_string(key.id)));
      parsed.keys.push_back(key);

    } else if (instruction == "exit") {
      ExitZone exit;
      if (!_ReadVec3(line, exit.Min) || !_ReadVec3(line, exit.Max) ||
          !_AtEnd(line))
        return (_Fail(error, lvlPath, lineNumber,
                      "exit needs exactly minX minY minZ maxX maxY maxZ"));
      if (exit.Min.x > exit.Max.x || exit.Min.y > exit.Max.y ||
          exit.Min.z > exit.Max.z)
        return (
            _Fail(error, lvlPath, lineNumber, "exit min is greater than max"));
      parsed.exit = exit;
      parsed.hasExit = true;

    } else if (instruction == "trap") {
      _PendingTrap trap{lineNumber, -1, 0};
      if (!_ReadInt(line, trap.brushIndex) ||
          !_ReadInt(line, trap.damagePerSecond) || !_AtEnd(line))
        return (_Fail(error, lvlPath, lineNumber,
                      "trap needs exactly brushIndex damagePerSecond"));
      if (trap.damagePerSecond < 0)
        return (_Fail(error, lvlPath, lineNumber,
                      "trap damage must be zero or greater"));
      traps.push_back(trap);

    } else if (instruction == "light") {
      Light light;
      if (!_ReadVec3(line, light.pos) || !_ReadVec3(line, light.color) ||
          !_ReadFloat(line, light.intensity) || !_AtEnd(line))
        return (_Fail(error, lvlPath, lineNumber,
                      "light needs exactly x y z r g b intensity"));
      parsed.lights.push_back(light);

    } else if (instruction == "enemy") {
      EnemySpawn enemy;
      if (!_ReadEnemyType(line, enemy.type) || !_ReadVec3(line, enemy.spawn))
        return (_Fail(error, lvlPath, lineNumber,
                      "enemy needs type x y z [x2 y2 z2]"));
      line >> std::ws;
      if (line.eof()) {
        enemy.pEnd = enemy.spawn;
      } else if (!_ReadVec3(line, enemy.pEnd) || !_AtEnd(line)) {
        return (_Fail(error, lvlPath, lineNumber,
                      "enemy needs type x y z [x2 y2 z2]"));
      } // end if
      parsed.enemies.push_back(enemy);

    } else {
      return (_Fail(error, lvlPath, lineNumber,
                    "unknown instruction '" + instruction + "'"));
    } // end if
  } // end while

  if (input.bad())
    return (_Fail(error, lvlPath, 0, "could not read the level file"));

  if (!hasPlayer)
    return (
        _Fail(error, lvlPath, 0, "the level does not define the player spawn"));

  if (!parsed.hasExit)
    return (_Fail(error, lvlPath, 0, "the level does not define an exit"));

  // Apply traps first so doors can detect a brush flagged as both.
  std::vector<bool> isTrap(parsed.brushes.size(), false);

  for (const _PendingTrap &trap : traps) {
    if (trap.brushIndex < 0 ||
        trap.brushIndex >= static_cast<int>(parsed.brushes.size()))
      return (_Fail(error, lvlPath, trap.line,
                    "trap references a brush that does not exist"));
    parsed.brushes[trap.brushIndex].trapDps = trap.damagePerSecond;
    isTrap[trap.brushIndex] = true;
  } // end for

  for (const _PendingDoor &door : doors) {
    if (door.brushIndex < 0 ||
        door.brushIndex >= static_cast<int>(parsed.brushes.size()))
      return (_Fail(error, lvlPath, door.line,
                    "door references a brush that does not exist"));
    if (door.keyId < 0)
      return (_Fail(error, lvlPath, door.line,
                    "door key id must be zero or greater"));
    if (keyIds.find(door.keyId) == keyIds.end())
      return (_Fail(error, lvlPath, door.line,
                    "door references undefined key id " +
                        std::to_string(door.keyId)));
    if (isTrap[door.brushIndex])
      return (_Fail(error, lvlPath, door.line,
                    "a brush cannot be both a door and a trap"));
    parsed.brushes[door.brushIndex].doorKey = door.keyId;
  } // end for

  // Move parsed level to the object that called the method
  *this = std::move(parsed);
  return (true);
}

// eof - Level.cxx
