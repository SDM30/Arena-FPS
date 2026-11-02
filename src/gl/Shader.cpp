// =========================================================================
// Shader program (OpenGL 3.3 core)
// =========================================================================

#include <glad/gl.h>

#include <gl/Shader.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

// -------------------------------------------------------------------------
gl::Shader::~Shader() { this->Destroy(); }

// -------------------------------------------------------------------------
gl::Shader::Shader(Shader &&other) noexcept : Program(other.Program) {
  other.Program = 0;
}

// -------------------------------------------------------------------------
gl::Shader &gl::Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    this->Destroy();
    this->Program = other.Program;
    other.Program = 0;
  } // end if
  return (*this);
}

// -------------------------------------------------------------------------
bool gl::Shader::LoadFromSource(const std::string &vertexSource,
                                const std::string &fragmentSource) {
  this->Destroy();

  unsigned int vertex = Self::_Compile(GL_VERTEX_SHADER, vertexSource);
  if (vertex == 0)
    return (false);

  unsigned int fragment = Self::_Compile(GL_FRAGMENT_SHADER, fragmentSource);
  if (fragment == 0) {
    glDeleteShader(vertex);
    return (false);
  } // end if

  this->Program = glCreateProgram();
  glAttachShader(this->Program, vertex);
  glAttachShader(this->Program, fragment);
  glLinkProgram(this->Program);

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  int ok = 0;
  glGetProgramiv(this->Program, GL_LINK_STATUS, &ok);
  if (ok == GL_FALSE) {
    int length = 0;
    glGetProgramiv(this->Program, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
    glGetProgramInfoLog(this->Program, length, nullptr, log.data());
    std::cerr << "Shader link error:\n" << log << std::endl;
    this->Destroy();
    return (false);
  } // end if

  return (true);
}

// -------------------------------------------------------------------------
bool gl::Shader::LoadFromFiles(const std::string &vertexFilename,
                               const std::string &fragmentFilename) {
  auto read = [](const std::string &fname, std::string &out) -> bool {
    std::ifstream file(fname, std::ios::in | std::ios::binary);
    if (!file.is_open())
      return (false);
    std::stringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return (true);
  };

  std::string vertexSource, fragmentSource;
  if (!read(vertexFilename, vertexSource)) {
    std::cerr << "Could not open shader: " << vertexFilename << std::endl;
    return (false);
  } // end if
  if (!read(fragmentFilename, fragmentSource)) {
    std::cerr << "Could not open shader: " << fragmentFilename << std::endl;
    return (false);
  } // end if

  return (this->LoadFromSource(vertexSource, fragmentSource));
}

// -------------------------------------------------------------------------
void gl::Shader::Destroy() {
  if (this->Program != 0) {
    glDeleteProgram(this->Program);
    this->Program = 0;
  } // end if
}

// -------------------------------------------------------------------------
void gl::Shader::Use() const { glUseProgram(this->Program); }

// -------------------------------------------------------------------------
void gl::Shader::SetFloat(const std::string &name, float value) const {
  glUniform1f(this->_GetUniformLocation(name), value);
}

// -------------------------------------------------------------------------
void gl::Shader::SetInt(const std::string &name, int value) const {
  glUniform1i(this->_GetUniformLocation(name), value);
}

// -------------------------------------------------------------------------
void gl::Shader::SetVec3(const std::string &name,
                         const glm::vec3 &value) const {
  glUniform3fv(this->_GetUniformLocation(name), 1, &value[0]);
}

// -------------------------------------------------------------------------
void gl::Shader::SetMat4(const std::string &name,
                         const glm::mat4 &value) const {
  glUniformMatrix4fv(this->_GetUniformLocation(name), 1, GL_FALSE,
                     &value[0][0]);
}

// -------------------------------------------------------------------------
unsigned int gl::Shader::_Compile(unsigned int type,
                                  const std::string &source) {
  unsigned int shader = glCreateShader(type);
  const char *code = source.c_str();
  glShaderSource(shader, 1, &code, nullptr);
  glCompileShader(shader);

  int ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok == GL_FALSE) {
    int length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    std::cerr << "Shader compile error ("
              << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
              << "):\n" << log << std::endl;
    glDeleteShader(shader);
    return (0);
  } // end if

  return (shader);
}

// -------------------------------------------------------------------------
int gl::Shader::_GetUniformLocation(const std::string &name) const {
  return (glGetUniformLocation(this->Program, name.c_str()));
}

// eof - Shader.cpp
