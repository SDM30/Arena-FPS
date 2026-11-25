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
    collider.box.min = brushes[i].min;
    collider.box.max = brushes[i].max;
    collider.brushIndex = static_cast<int>(i);
    collider.enabled = true;
    this->Colliders.push_back(collider);
  } // end for
}

// -------------------------------------------------------------------------
void physics::CollisionWorld::SetEnabled(int brushIndex, bool enabled) {
  if (brushIndex < 0 ||
      brushIndex >= static_cast<int>(this->Colliders.size()))
    return;
  this->Colliders[static_cast<std::size_t>(brushIndex)].enabled = enabled;
}

// -------------------------------------------------------------------------
physics::MoveResult physics::CollisionWorld::Move(
    Body &body, const glm::vec3 &delta) const {
  MoveResult result;

  // X
  if (delta.x != 0.0f) {
    body.position.x += delta.x;
    float resolved = body.position.x;
    bool hit = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.enabled ||
          !Overlaps(BodyAabb(body), collider.box))
        continue;

      if (delta.x > 0.0f) {
        resolved = std::min(resolved, collider.box.min.x -
                                          body.halfExtents.x);
      } else {
        resolved = std::max(resolved, collider.box.max.x +
                                          body.halfExtents.x);
      } // end if
      hit = true;

      Contact contact;
      contact.brushIndex = collider.brushIndex;
      contact.normal = glm::vec3((delta.x > 0.0f) ? -1.0f : 1.0f, 0.0f, 0.0f);
      // how far the body has to move to stop overlapping
      contact.depth = std::abs(body.position.x - resolved);
      result.contacts.push_back(contact);
    } // end for

    if (hit) {
      body.position.x = resolved;
      body.velocity.x = 0.0f;
      result.hitWall = true;
    } // end if
  } // end if

  // Z
  if (delta.z != 0.0f) {
    body.position.z += delta.z;
    float resolved = body.position.z;
    bool hit = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.enabled ||
          !Overlaps(BodyAabb(body), collider.box))
        continue;

      if (delta.z > 0.0f) {
        resolved = std::min(resolved, collider.box.min.z -
                                          body.halfExtents.z);
      } else {
        resolved = std::max(resolved, collider.box.max.z +
                                          body.halfExtents.z);
      } // end if
      hit = true;

      Contact contact;
      contact.brushIndex = collider.brushIndex;
      contact.normal = glm::vec3(0.0f, 0.0f, (delta.z > 0.0f) ? -1.0f : 1.0f);
      contact.depth = std::abs(body.position.z - resolved);
      result.contacts.push_back(contact);
    } // end for

    if (hit) {
      body.position.z = resolved;
      body.velocity.z = 0.0f;
      result.hitWall = true;
    } // end if
  } // end if

  // Y
  if (delta.y != 0.0f) {
    body.position.y += delta.y;
    float resolved = body.position.y;
    bool ground = false;
    bool ceiling = false;

    for (const Collider &collider : this->Colliders) {
      if (!collider.enabled ||
          !Overlaps(BodyAabb(body), collider.box))
        continue;

      if (delta.y <= 0.0f) {
        resolved = std::max(resolved, collider.box.max.y +
                                          body.halfExtents.y);
        ground = true;

        Contact contact;
        contact.brushIndex = collider.brushIndex;
        contact.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        contact.depth = std::abs(body.position.y - resolved);
        result.contacts.push_back(contact);
      } else {
        resolved = std::min(resolved, collider.box.min.y -
                                          body.halfExtents.y);
        ceiling = true;

        Contact contact;
        contact.brushIndex = collider.brushIndex;
        contact.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        contact.depth = std::abs(body.position.y - resolved);
        result.contacts.push_back(contact);
      } // end if
    } // end for

    body.position.y = resolved;
    if (ground || ceiling)
      body.velocity.y = 0.0f;
    result.hitGround = ground;
    result.hitCeiling = ceiling;
  } // end if

  body.onGround = result.hitGround;
  return (result);
}

// -------------------------------------------------------------------------
physics::RayHit physics::CollisionWorld::Raycast(const Ray &ray) const {
  RayHit nearest;
  bool found = false;

  for (const Collider &collider : this->Colliders) {
    if (!collider.enabled)
      continue;

    RayHit candidate;
    if (!this->RaycastAabb(ray, collider.box, candidate))
      continue;

    candidate.brushIndex = collider.brushIndex;
    candidate.hit = true;

    if (!found || candidate.distance < nearest.distance) {
      nearest = candidate;
      found = true;
    } // end if
  } // end for

  return (nearest);
}

// -------------------------------------------------------------------------
bool physics::CollisionWorld::RaycastAabb(const Ray &ray, const Aabb &box,
                                          RayHit &hit) const {
  float tMin = 0.0f;
  float tMax = ray.maxDistance;
  int hitAxis = -1;

  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (std::abs(ray.direction[axis]) < 1e-6f) {
      if (ray.origin[axis] < box.min[axis] ||
          ray.origin[axis] > box.max[axis])
        return (false);
    } else {
      // Parametric distances where the ray enters and exits this slab.
      float t1 = (box.min[axis] - ray.origin[axis]) / ray.direction[axis];
      float t2 = (box.max[axis] - ray.origin[axis]) / ray.direction[axis];
      // If Direction.axis < 0 the two values come  out reversed
      if (t1 > t2)
        std::swap(t1, t2);

      if (t1 > tMin) {
        tMin = t1;
        hitAxis = static_cast<int>(axis);
      } // end if
      tMax = std::min(tMax, t2);

      if (tMin > tMax)
        return (false);
    } // end if
  } // end for

  hit.hit = true;
  hit.point = ray.origin + ray.direction * tMin;
  hit.distance = tMin;
  hit.normal = glm::vec3(0.0f);

  if (hitAxis >= 0) {
    const int axis = hitAxis;
    hit.normal[axis] = (ray.direction[axis] > 0.0f) ? -1.0f : 1.0f;
  } else {
    // Origin is already inside the box: push back against the ray.
    hit.normal = -ray.direction;
  } // end if

  return (true);
}
// eof - CollisionWorld.cxx
