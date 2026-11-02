// =========================================================================
// Indexed triangle mesh (VAO / VBO / EBO)
// =========================================================================

#include <glad/gl.h>

#include <gl/Mesh.h>

#include <iostream>
#include <utility>

// -------------------------------------------------------------------------
gl::Mesh::~Mesh() { this->Destroy(); }

// -------------------------------------------------------------------------
gl::Mesh::Mesh(Mesh &&other) noexcept
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
      IndexCount(other.IndexCount) {
  other.VAO = other.VBO = other.EBO = 0;
  other.IndexCount = 0;
}

// -------------------------------------------------------------------------
gl::Mesh &gl::Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    this->Destroy();
    this->VAO = other.VAO;
    this->VBO = other.VBO;
    this->EBO = other.EBO;
    this->IndexCount = other.IndexCount;
    other.VAO = other.VBO = other.EBO = 0;
    other.IndexCount = 0;
  } // end if
  return (*this);
}

// -------------------------------------------------------------------------
bool gl::Mesh::Upload(const std::vector<float> &positions,
                      const std::vector<float> &normals,
                      const std::vector<unsigned int> &indices) {
  const bool positionsOk =
      (!positions.empty()) && ((positions.size() % 3) == 0);
  const bool normalsOk = normals.empty() ||
                         (normals.size() == positions.size());
  if (!positionsOk || !normalsOk || indices.empty()) {
    std::cerr << "Mesh::Upload: inconsistent vertex data" << std::endl;
    return (false);
  } // end if

  const std::size_t vertexCount = positions.size() / 3;
  std::vector<float> vertices;
  vertices.reserve(vertexCount * 6);
  for (std::size_t i = 0; i < vertexCount; ++i) {
    vertices.push_back(positions[(i * 3) + 0]);
    vertices.push_back(positions[(i * 3) + 1]);
    vertices.push_back(positions[(i * 3) + 2]);
    if (normals.empty()) {
      vertices.push_back(0.0f);
      vertices.push_back(0.0f);
      vertices.push_back(0.0f);
    } else {
      vertices.push_back(normals[(i * 3) + 0]);
      vertices.push_back(normals[(i * 3) + 1]);
      vertices.push_back(normals[(i * 3) + 2]);
    } // end if
  } // end for

  this->Destroy();

  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->EBO);

  glBindVertexArray(this->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() *
                                       sizeof(unsigned int)),
               indices.data(), GL_STATIC_DRAW);

  const GLsizei stride = static_cast<GLsizei>(6 * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<void *>(3 * sizeof(float)));

  glBindVertexArray(0);

  this->IndexCount = static_cast<int>(indices.size());
  return (true);
}

// -------------------------------------------------------------------------
void gl::Mesh::Draw() const {
  if (this->VAO != 0) {
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->IndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  } // end if
}

// -------------------------------------------------------------------------
void gl::Mesh::Destroy() {
  if (this->EBO != 0) {
    glDeleteBuffers(1, &this->EBO);
    this->EBO = 0;
  } // end if
  if (this->VBO != 0) {
    glDeleteBuffers(1, &this->VBO);
    this->VBO = 0;
  } // end if
  if (this->VAO != 0) {
    glDeleteVertexArrays(1, &this->VAO);
    this->VAO = 0;
  } // end if
  this->IndexCount = 0;
}

// eof - Mesh.cpp
