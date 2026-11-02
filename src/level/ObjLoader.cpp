// =========================================================================
// Wavefront OBJ loader (tinyobjloader)
// =========================================================================

#include <level/ObjLoader.h>

#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <cmath>
#include <cstddef>

namespace {
// -------------------------------------------------------------------------
glm::vec3 _FaceNormal(const glm::vec3 &a, const glm::vec3 &b,
                      const glm::vec3 &c) {
  const glm::vec3 normal = glm::cross(b - a, c - a);
  const float length = glm::length(normal);
  if (length <= 1e-8f)
    return (glm::vec3(0.0f, 1.0f, 0.0f));
  return (normal / length);
}

// -------------------------------------------------------------------------
bool _PositionAt(const tinyobj::attrib_t &attrib,
                 const tinyobj::index_t &index, glm::vec3 &position) {
  if (index.vertex_index < 0)
    return (false);
  const std::size_t at = static_cast<std::size_t>(index.vertex_index) * 3;
  if (at + 2 >= attrib.vertices.size())
    return (false);
  position = glm::vec3(attrib.vertices[at], attrib.vertices[at + 1],
                       attrib.vertices[at + 2]);
  return (true);
}
} // namespace

// -------------------------------------------------------------------------
bool level::LoadObj(const std::string &filename, ObjMesh &mesh,
                    std::string *error) {
  auto fail = [error](const std::string &message) -> bool {
    if (error != nullptr)
      *error = message;
    return (false);
  };

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string loadError;

  mesh = ObjMesh{};

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &loadError,
                        filename.c_str(), nullptr, true)) {
    if (loadError.empty())
      loadError = "could not open '" + filename + "'";
    return (fail(loadError));
  } // end if

  for (const tinyobj::shape_t &shape : shapes) {
    const tinyobj::mesh_t &source = shape.mesh;
    std::size_t indexOffset = 0;

    for (std::size_t face = 0; face < source.num_face_vertices.size();
         ++face) {
      const std::size_t faceVertexCount =
          static_cast<std::size_t>(source.num_face_vertices[face]);

      if (faceVertexCount < 3) {
        indexOffset += faceVertexCount;
        continue;
      } // end if

      // Fallback normal for OBJ files without 'vn' entries.
      glm::vec3 faceNormal(0.0f, 1.0f, 0.0f);
      glm::vec3 p0, p1, p2;
      if (indexOffset + 2 < source.indices.size() &&
          _PositionAt(attrib, source.indices[indexOffset], p0) &&
          _PositionAt(attrib, source.indices[indexOffset + 1], p1) &&
          _PositionAt(attrib, source.indices[indexOffset + 2], p2))
        faceNormal = _FaceNormal(p0, p1, p2);

      const unsigned int base =
          static_cast<unsigned int>(mesh.Positions.size() / 3);

      for (std::size_t v = 0; v < faceVertexCount; ++v) {
        if (indexOffset + v >= source.indices.size())
          return (fail("truncated index buffer in '" + filename + "'"));

        const tinyobj::index_t &index = source.indices[indexOffset + v];
        glm::vec3 position(0.0f);
        if (!_PositionAt(attrib, index, position))
          return (fail("invalid vertex index in '" + filename + "'"));

        glm::vec3 normal = faceNormal;
        if (index.normal_index >= 0) {
          const std::size_t at =
              static_cast<std::size_t>(index.normal_index) * 3;
          if (at + 2 >= attrib.normals.size())
            return (fail("invalid normal index in '" + filename + "'"));
          normal = glm::vec3(attrib.normals[at], attrib.normals[at + 1],
                             attrib.normals[at + 2]);
        } // end if

        mesh.Positions.push_back(position.x);
        mesh.Positions.push_back(position.y);
        mesh.Positions.push_back(position.z);
        mesh.Normals.push_back(normal.x);
        mesh.Normals.push_back(normal.y);
        mesh.Normals.push_back(normal.z);
      } // end for

      // Fan triangulation (tinyobjloader usually returns triangles already).
      for (std::size_t v = 1; v + 1 < faceVertexCount; ++v) {
        mesh.Indices.push_back(base);
        mesh.Indices.push_back(base + static_cast<unsigned int>(v));
        mesh.Indices.push_back(base + static_cast<unsigned int>(v + 1));
      } // end for

      indexOffset += faceVertexCount;
    } // end for
  } // end for

  if (mesh.Positions.empty() || mesh.Indices.empty())
    return (fail("'" + filename + "' contains no triangle geometry"));

  return (true);
}

// eof - ObjLoader.cpp
