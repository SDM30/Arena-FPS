// =========================================================================
// Stores world state
// =========================================================================
#ifndef __game__World__h__
#define __game__World__h__
#include <vector>

namespace game {
struct World {
  std::vector<bool> doorOpen;
  std::vector<bool> keyTaken;
  std::vector<int> keysHeld;
};
} // namespace game

#endif // __level__ObjLoader__h__

// eof - ObjLoader.h