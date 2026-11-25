// =========================================================================
// Static collision world and axis-separated body resolution
// =========================================================================

#include <physics/CollisionWorld.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

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
physics::MoveResult physics::CollisionWorld::Move(
    Body &body, const glm::vec3 &delta) const {
  MoveResult result;

  // X
  if (delta.x != 0.0f) {
    body.Position.x += delta.x;
    float resolved = body.Position.x;
    bool hit = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.Enabled ||
          !Overlaps(BodyAabb(body), collider.Box))
        continue;

      if (delta.x > 0.0f) {
        resolved = std::min(resolved, collider.Box.Min.x -
                                          body.HalfExtents.x);
      } else {
        resolved = std::max(resolved, collider.Box.Max.x +
                                          body.HalfExtents.x);
      } // end if
      hit = true;

      Contact contact;
      contact.BrushIndex = collider.BrushIndex;
      contact.Normal = glm::vec3((delta.x > 0.0f) ? -1.0f : 1.0f, 0.0f, 0.0f);
      // how far the body has to move to stop overlapping
      contact.Depth = std::abs(body.Position.x - resolved);
      result.Contacts.push_back(contact);
    } // end for

    if (hit) {
      body.Position.x = resolved;
      body.Velocity.x = 0.0f;
      result.HitWall = true;
    } // end if
  } // end if

  // Z
  if (delta.z != 0.0f) {
    body.Position.z += delta.z;
    float resolved = body.Position.z;
    bool hit = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.Enabled ||
          !Overlaps(BodyAabb(body), collider.Box))
        continue;

      if (delta.z > 0.0f) {
        resolved = std::min(resolved, collider.Box.Min.z -
                                          body.HalfExtents.z);
      } else {
        resolved = std::max(resolved, collider.Box.Max.z +
                                          body.HalfExtents.z);
      } // end if
      hit = true;

      Contact contact;
      contact.BrushIndex = collider.BrushIndex;
      contact.Normal = glm::vec3(0.0f, 0.0f, (delta.z > 0.0f) ? -1.0f : 1.0f);
      contact.Depth = std::abs(body.Position.z - resolved);
      result.Contacts.push_back(contact);
    } // end for

    if (hit) {
      body.Position.z = resolved;
      body.Velocity.z = 0.0f;
      result.HitWall = true;
    } // end if
  } // end if

  // Y
  if (delta.y != 0.0f) {
    body.Position.y += delta.y;
    float resolved = body.Position.y;
    bool ground = false;
    bool ceiling = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.Enabled ||
          !Overlaps(BodyAabb(body), collider.Box))
        continue;

      if (delta.y <= 0.0f) {
        resolved = std::max(resolved, collider.Box.Max.y +
                                          body.HalfExtents.y);
        ground = true;

        Contact contact;
        contact.BrushIndex = collider.BrushIndex;
        contact.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        contact.Depth = std::abs(body.Position.y - resolved);
        result.Contacts.push_back(contact);
      } else {
        resolved = std::min(resolved, collider.Box.Min.y -
                                          body.HalfExtents.y);
        ceiling = true;

        Contact contact;
        contact.BrushIndex = collider.BrushIndex;
        contact.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
        contact.Depth = std::abs(body.Position.y - resolved);
        result.Contacts.push_back(contact);
      } // end if
    } // end for

    body.Position.y = resolved;
    if (ground || ceiling)
      body.Velocity.y = 0.0f;
    result.HitGround = ground;
    result.HitCeiling = ceiling;
  } // end if

  body.OnGround = result.HitGround;
  return (result);
}

// eof - CollisionWorld.cxx
