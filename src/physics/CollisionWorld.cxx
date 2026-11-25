// =========================================================================
// Static collision world and vertical body resolution
// =========================================================================

#include <physics/CollisionWorld.h>

#include <cstddef>

namespace {
// -------------------------------------------------------------------------
bool _Overlaps(const physics::Body &body, const physics::Aabb &box) {
  return (body.Position.x - body.HalfExtents.x < box.Max.x &&
          body.Position.x + body.HalfExtents.x > box.Min.x &&
          body.Position.y - body.HalfExtents.y < box.Max.y &&
          body.Position.y + body.HalfExtents.y > box.Min.y &&
          body.Position.z - body.HalfExtents.z < box.Max.z &&
          body.Position.z + body.HalfExtents.z > box.Min.z);
}
} // namespace

// -------------------------------------------------------------------------
void physics::CollisionWorld::Build(
    const std::vector<level::Brush> &brushes) {
  this->Colliders.clear();
  this->Colliders.reserve(brushes.size());

  for (std::size_t i = 0; i < brushes.size(); ++i) {
    Collider collider;
    collider.Box.Min = brushes[i].Min;
    collider.Box.Max = brushes[i].Max;
    collider.BrushIndex = static_cast<int>(i);
    collider.Enabled = true;
    this->Colliders.push_back(collider);
  } // end for
}

// -------------------------------------------------------------------------
void physics::CollisionWorld::SetEnabled(int brushIndex, bool enabled) {
  if (brushIndex < 0 ||
      brushIndex >= static_cast<int>(this->Colliders.size()))
    return;
  this->Colliders[static_cast<std::size_t>(brushIndex)].Enabled = enabled;
}

// -------------------------------------------------------------------------
physics::MoveResult physics::CollisionWorld::MoveBody(Body &body,
                                                      float dt) const {
  MoveResult result;
  const glm::vec3 previous = body.Position;

  body.Velocity.y += kGravity * dt;
  if (body.Velocity.y < -kMaxFallSpeed)
    body.Velocity.y = -kMaxFallSpeed;
  body.Position.y += body.Velocity.y * dt;

  body.OnGround = false;

  constexpr float kGroundEpsilon = 1e-4f;
  const float previousBottom = previous.y - body.HalfExtents.y;
  const float previousTop = previous.y + body.HalfExtents.y;

  for (const Collider &collider : this->Colliders) {
    if (!collider.Enabled || !_Overlaps(body, collider.Box))
      continue;

    if (body.Velocity.y <= 0.0f &&
        previousBottom >= collider.Box.Max.y - kGroundEpsilon) {
      // Land on top of the brush.
      body.Position.y = collider.Box.Max.y + body.HalfExtents.y;
      body.Velocity.y = 0.0f;
      body.OnGround = true;
      result.HitGround = true;

      Contact contact;
      contact.BrushIndex = collider.BrushIndex;
      contact.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
      contact.Depth = collider.Box.Max.y - (previous.y - body.HalfExtents.y);
      result.Contacts.push_back(contact);
    } else if (body.Velocity.y > 0.0f &&
               previousTop <= collider.Box.Min.y + kGroundEpsilon) {
      // Hit the underside of a brush.
      body.Position.y = collider.Box.Min.y - body.HalfExtents.y;
      body.Velocity.y = 0.0f;
      result.HitCeiling = true;

      Contact contact;
      contact.BrushIndex = collider.BrushIndex;
      contact.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
      contact.Depth = (previous.y + body.HalfExtents.y) - collider.Box.Min.y;
      result.Contacts.push_back(contact);
    } // end if
  } // end for

  return (result);
}

// eof - CollisionWorld.cxx
