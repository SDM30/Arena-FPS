// =========================================================================
// Shader program (OpenGL 3.3 core)
// =========================================================================
#ifndef __gl__Shader__h__
#define __gl__Shader__h__

#include <glm/glm.hpp>
#include <string>

namespace gl {
/**
 * Owns a linked GLSL program. The destructor releases the GL resource, so a
 * valid context must be current when the object goes out of scope.
 */
class Shader {
public:
  using Self = Shader;

public:
  Shader() = default;
  ~Shader();

  Shader(const Self &) = delete;
  Self &operator=(const Self &) = delete;
  Shader(Shader &&other) noexcept;
  Shader &operator=(Shader &&other) noexcept;

  bool LoadFromSource(const std::string &vertexSource,
                      const std::string &fragmentSource);
  bool LoadFromFiles(const std::string &vertexFilename,
                     const std::string &fragmentFilename);
  void Destroy();

  void Use() const;

  bool IsValid() const { return (this->Program != 0); }
  unsigned int GetId() const { return (this->Program); }

  void SetFloat(const std::string &name, float value) const;
  void SetInt(const std::string &name, int value) const;
  void SetVec3(const std::string &name, const glm::vec3 &value) const;
  void SetMat4(const std::string &name, const glm::mat4 &value) const;

private:
  static unsigned int _Compile(unsigned int type, const std::string &source);
  int _GetUniformLocation(const std::string &name) const;

private:
  unsigned int Program{0};
};
} // namespace gl

#endif // __gl__Shader__h__

// eof - Shader.h
