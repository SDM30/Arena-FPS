// =========================================================================
// Wavefront OBJ loader (positions, normals, triangles)
// =========================================================================
#ifndef __level__ObjLoader__h__
#define __level__ObjLoader__h__

#include <string>
#include <vector>

namespace level {
/**
 * Geometry-only representation of an OBJ file. Positions and normals are
 * flat xyz arrays (one normal per position) ready for gl::Mesh::Upload.
 */
struct ObjMesh {
  std::vector<float> Positions;
  std::vector<float> Normals;
  std::vector<unsigned int> Indices;

  bool IsEmpty() const { return (this->Indices.empty()); }
};

/**
 * Loads the first geometry found in an OBJ file. Polygons are fan
 * triangulated and faces without normals get a flat face normal.
 * Returns false and fills "error" (when provided) on failure.
 */
bool LoadObj(const std::string &filename, ObjMesh &mesh,
             std::string *error = nullptr);
} // namespace level

#endif // __level__ObjLoader__h__

// eof - ObjLoader.h
