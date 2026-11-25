// =========================================================================
// Interaction between bodies and the static level geometry
// =========================================================================
#ifndef __physics__CollisionWorld__h__
#define __physics__CollisionWorld__h__

#include <level/Level.h>
#include <physics/Physics.h>

#include <vector>

namespace physics {
class CollisionWorld {
public:
  // Copies the level brushes into collision-only AABBs. All colliders start
  // enabled; doors can disable theirs later.
  void Build(const std::vector<level::Brush> &brushes);
  void SetEnabled(int brushIndex, bool enabled);

  // Resolves one displacement against the static colliders, axis by axis.
  // Movement policy (gravity, friction, jump) belongs to QPlayerMovement.
  MoveResult Move(Body &body, const glm::vec3 &delta) const;

private:
  std::vector<Collider> Colliders;
};
} // namespace physics

#endif // __physics__CollisionWorld__h__

// eof - CollisionWorld.h
