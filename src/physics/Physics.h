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
  glm::vec3 Position{0.0f};
  glm::vec3 Velocity{0.0f};
  // Body dimensions from the center to each face.
  glm::vec3 HalfExtents{0.3f, 0.9f, 0.3f};
  bool OnGround = false;
};

// Static world geometry derived from the level brushes.
struct Aabb {
  glm::vec3 Min{0.0f};
  glm::vec3 Max{0.0f};
};

struct Collider {
  Aabb Box;
  int BrushIndex = -1;
  // Doors disable their brush when opened.
  bool Enabled = true;
};

// Transient collision result. Valid only for the step that produced it.
struct Contact {
  int BrushIndex = -1;
  glm::vec3 Normal{0.0f};
  float Depth = 0.0f;
};

struct MoveResult {
  bool HitGround = false;
  bool HitCeiling = false;
  bool HitWall = false;
  // Populated as resolution starts producing more than one contact. Axis
  // separation currently reports the flags above.
  std::vector<Contact> Contacts;
};
} // namespace physics

#endif // __physics__Physics__h__

// eof - Physics.h
