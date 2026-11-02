// =========================================================================
// Indexed triangle mesh (VAO / VBO / EBO)
// =========================================================================
#ifndef __gl__Mesh__h__
#define __gl__Mesh__h__

#include <vector>

namespace gl {
/**
 * Static indexed mesh. Vertex layout (locations set in the shader):
 *   location 0 -> position (vec3)
 *   location 1 -> normal   (vec3)
 *
 * Positions and normals are passed as flat xyz arrays; Upload interleaves
 * them into a single VBO. The destructor releases the GL resources, so a
 * valid context must be current when the object goes out of scope.
 */
class Mesh {
public:
  using Self = Mesh;

public:
  Mesh() = default;
  ~Mesh();

  Mesh(const Self &) = delete;
  Self &operator=(const Self &) = delete;
  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &&other) noexcept;

  bool Upload(const std::vector<float> &positions,
              const std::vector<float> &normals,
              const std::vector<unsigned int> &indices);
  void Draw() const;
  void Destroy();

  bool IsValid() const { return (this->VAO != 0); }
  int GetIndexCount() const { return (this->IndexCount); }

private:
  unsigned int VAO{0};
  unsigned int VBO{0};
  unsigned int EBO{0};
  int IndexCount{0};
};
} // namespace gl

#endif // __gl__Mesh__h__

// eof - Mesh.h
