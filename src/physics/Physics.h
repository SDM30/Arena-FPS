// =========================================================================
// Simple world kinematics and collision types
// =========================================================================
#ifndef __physics__Physics__h__
#define __physics__Physics__h__

#include <glm/glm.hpp>

#include <vector>

namespace physics {
// Arena tuning: real gravity is -9.81; Quake's scaled gravity is about -20.
constexpr float kGravity = -20.0f;

// Rigid rectangular body for the player and, later, enemies and props.
struct Body {
  // Center of the axis-aligned bounding box.
  glm::vec3 position{0.0f};
  glm::vec3 velocity{0.0f};
  // Body dimensions from the center to each face.
  glm::vec3 halfExtents{0.3f, 0.9f, 0.3f};
  bool onGround = false;
};

// Static world geometry derived from the level brushes.
struct Aabb {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};
};

// Body center and half-extents converted to its bounding box.
inline Aabb BodyAabb(const Body &body) {
  Aabb box;
  box.min = body.position - body.halfExtents;
  box.max = body.position + body.halfExtents;
  return (box);
}

// Grows a box uniformly on every axis.
inline Aabb Expanded(const Aabb &box, float margin) {
  Aabb expanded;
  expanded.min = box.min - glm::vec3(margin);
  expanded.max = box.max + glm::vec3(margin);
  return (expanded);
}

// Strict overlap: touching faces alone do not count.
inline bool Overlaps(const Aabb &a, const Aabb &b) {
  return (a.min.x < b.max.x && a.max.x > b.min.x &&
          a.min.y < b.max.y && a.max.y > b.min.y &&
          a.min.z < b.max.z && a.max.z > b.min.z);
}

struct Collider {
  Aabb box;
  int brushIndex = -1;
  // Doors disable their brush when opened.
  bool enabled = true;
};

// Transient collision result. Valid only for the step that produced it.
struct Contact {
  int brushIndex = -1;
  glm::vec3 normal{0.0f};
  float depth = 0.0f;
};

struct MoveResult {
  bool hitGround = false;
  bool hitCeiling = false;
  bool hitWall = false;
  // Populated as resolution starts producing more than one contact. Axis
  // separation currently reports the flags above.
  std::vector<Contact> contacts;
};

struct Ray {
  glm::vec3 origin{0.0f};
  // Normalized
  glm::vec3 direction{0.0f, 0.0f, -1.0};
  float maxDistance = 100.0f;
};

struct RayHit {
  bool hit = false;
  float  distance = 0.0f;
  glm::vec3 point{0.0f};
  glm::vec3 normal{0.0f};
  int brushIndex = -1;
};

} // namespace physics

#endif // __physics__Physics__h__

// eof - Physics.h
