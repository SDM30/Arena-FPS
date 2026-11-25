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

  // Get the nearest brush the player is looking at
  RayHit Raycast(const Ray &ray) const;

  // Computes the slab method
  bool RaycastAabb(const Ray &ray, const Aabb &box, RayHit &hit) const;

private:
  std::vector<Collider> Colliders;
};
} // namespace physics

#endif // __physics__CollisionWorld__h__

// eof - CollisionWorld.h
