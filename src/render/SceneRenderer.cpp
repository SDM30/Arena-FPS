// =========================================================================
// Scene rendering: level mesh, world entities and debug overlays
// =========================================================================

#include <render/SceneRenderer.h>

#include <glad/gl.h>

#include <game/World.h>
#include <gl/Camera.h>
#include <level/Level.h>

#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <vector>

namespace {
const char *kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vWorldPos;

void main() {
  vec4 world = model * vec4(aPosition, 1.0);
  vWorldPos = world.xyz;
  vNormal = mat3(transpose(inverse(model))) * aNormal;
  gl_Position = projection * view * world;
}
)";

const char *kFragmentShader = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 color;
uniform int unlit;
uniform float alpha;

// Point light taken from the first "light" line of the level. When the level
// defines none, the renderer binds a default fill light above the scene.
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

out vec4 fragmentColor;

void main() {
  // Debug markers (the light itself) are drawn at full brightness.
  if (unlit != 0) {
    fragmentColor = vec4(color, alpha);
    return;
  } // end if

  // The level mesh is a closed shell viewed from the inside; Blender exports
  // its normals pointing outward. Flip them on back faces so interior
  // surfaces receive light without enabling face culling.
  vec3 normal = normalize(vNormal);
  if (!gl_FrontFacing)
    normal = -normal;

  vec3 toLight = lightPos - vWorldPos;
  float dist = length(toLight);
  vec3 lightDirection = toLight / max(dist, 1e-4);

  float diffuse = max(dot(normal, lightDirection), 0.0);
  float attenuation = 1.0 / (1.0 + 0.02 * dist * dist);

  vec3 ambient = color * 0.20;
  vec3 direct =
      color * lightColor * lightIntensity * diffuse * attenuation;
  fragmentColor = vec4(ambient + direct, alpha);
}
)";

// -------------------------------------------------------------------------
bool _Fail(std::string *error, const char *message) {
  if (error != nullptr)
    *error = message;
  return (false);
}

// -------------------------------------------------------------------------
// Unit cube centered at the origin. Each face has its own four vertices so
// that the normals stay flat. Winding is counter-clockwise from outside.
void _MakeCube(std::vector<float> &positions, std::vector<float> &normals,
               std::vector<unsigned int> &indices) {
  constexpr float h = 0.5f;
  struct Face {
    glm::vec3 normal;
    glm::vec3 corners[4];
  };

  const Face faces[6] = {
      // +X
      {{1, 0, 0}, {{h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}}},
      // -X
      {{-1, 0, 0}, {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}}},
      // +Y
      {{0, 1, 0}, {{-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}}},
      // -Y
      {{0, -1, 0}, {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}}},
      // +Z
      {{0, 0, 1}, {{-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}}},
      // -Z
      {{0, 0, -1}, {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}}}};

  positions.clear();
  normals.clear();
  indices.clear();
  for (unsigned int face = 0; face < 6; ++face) {
    const unsigned int base = static_cast<unsigned int>(positions.size() / 3);
    for (unsigned int corner = 0; corner < 4; ++corner) {
      const glm::vec3 &p = faces[face].corners[corner];
      positions.push_back(p.x);
      positions.push_back(p.y);
      positions.push_back(p.z);
      normals.push_back(faces[face].normal.x);
      normals.push_back(faces[face].normal.y);
      normals.push_back(faces[face].normal.z);
    } // end for
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
  } // end for
}
} // namespace

// -------------------------------------------------------------------------
bool render::SceneRenderer::Initialize(const level::Level &level,
                                       std::string *error) {
  if (error != nullptr)
    error->clear();

  if (!this->Shader.LoadFromSource(kVertexShader, kFragmentShader))
    return (_Fail(error, "could not build the scene shader"));

  std::vector<float> positions, normals;
  std::vector<unsigned int> indices;
  _MakeCube(positions, normals, indices);
  if (!this->Cube.Upload(positions, normals, indices))
    return (_Fail(error, "could not upload the cube mesh"));

  if (!this->LoadLevel(level, error))
    return (false);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
  return (true);
}

// -------------------------------------------------------------------------
bool render::SceneRenderer::LoadLevel(const level::Level &level,
                                      std::string *error) {
  if (error != nullptr)
    error->clear();

  if (!this->LevelMesh.Upload(level.mesh.positions, level.mesh.normals,
                              level.mesh.indices))
    return (_Fail(error, "could not upload the level mesh"));

  return (true);
}

// -------------------------------------------------------------------------
void render::SceneRenderer::Draw(const gl::Camera &camera,
                                 const level::Level &level,
                                 const game::World &world, int width,
                                 int height,
                                 const render::Options &options) const {
  glViewport(0, 0, width, (height > 0) ? height : 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  this->Shader.Use();

  // Bind the level's point light. Without a "light" line in the level, use a
  // neutral fill light above the scene.
  if (!level.lights.empty()) {
    const level::Light &light = level.lights.front();
    this->Shader.SetVec3("lightPos", light.pos);
    this->Shader.SetVec3("lightColor", light.color);
    this->Shader.SetFloat("lightIntensity", light.intensity);
  } else {
    this->Shader.SetVec3("lightPos", glm::vec3(0.0f, 10.0f, 0.0f));
    this->Shader.SetVec3("lightColor", glm::vec3(1.0f));
    this->Shader.SetFloat("lightIntensity", 1.0f);
  } // end if

  this->Shader.SetMat4("model",
                       glm::mat4(1.0f)); // level is authored in world coords
  this->Shader.SetMat4("view", camera.GetViewMatrix());
  this->Shader.SetMat4("projection", camera.GetProjectionMatrix(width, height));
  this->Shader.SetVec3("color", glm::vec3(0.55f, 0.57f, 0.60f));
  this->Shader.SetInt("unlit", 0);
  this->Shader.SetFloat("alpha", 1.0f);

  this->LevelMesh.Draw();

  this->_DrawTraps(level);
  this->_DrawDoors(level, world);
  this->_DrawKeys(level, world);

  if (options.showCollision)
    this->_DrawBrushes(level, world);

  this->_DrawExit(level);
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawBox(const glm::vec3 &min,
                                     const glm::vec3 &max,
                                     const glm::vec3 &color, bool wireframe,
                                     float alpha) const {
  const glm::vec3 center = (min + max) * 0.5f;
  const glm::vec3 size = max - min;

  glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
  model = glm::scale(model, size);
  this->Shader.SetMat4("model", model);
  this->Shader.SetVec3("color", color);
  this->Shader.SetInt("unlit", 1);
  this->Shader.SetFloat("alpha", alpha);

  if (wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  this->Cube.Draw();
  if (wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawTraps(const level::Level &level) const {
  // Draw the exposed face of the brush. The rest of the box is hidden inside
  // the level geometry; the polygon offset avoids z-fighting with the
  // coplanar floor or wall surface.
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-1.0f, -1.0f);

  for (std::size_t i = 0; i < level.brushes.size(); ++i) {
    const level::Brush &brush = level.brushes[i];
    if (brush.trapDps <= 0)
      continue;
    this->_DrawBox(brush.min, brush.max, glm::vec3(1.0f, 0.2f, 0.2f));
  } // end for

  glDisable(GL_POLYGON_OFFSET_FILL);
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawDoors(const level::Level &level,
                                       const game::World &world) const {
  for (std::size_t i = 0; i < level.brushes.size(); ++i) {
    const level::Brush &brush = level.brushes[i];
    if (brush.doorKey < 0 || world.doorOpen[i])
      continue;
    this->_DrawBox(brush.min, brush.max, glm::vec3(0.2f, 0.45f, 1.0f));
  } // end for
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawKeys(const level::Level &level,
                                      const game::World &world) const {
  const glm::vec3 half(0.15f);
  for (std::size_t i = 0; i < level.keys.size(); ++i) {
    if (world.keyTaken[i])
      continue;
    this->_DrawBox(level.keys[i].pos - half, level.keys[i].pos + half,
                   glm::vec3(1.0f, 0.85f, 0.1f));
  } // end for
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawBrushes(const level::Level &level,
                                         const game::World &world) const {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_DEPTH_TEST);
  this->Shader.SetInt("unlit", 1);
  this->Shader.SetVec3("color", glm::vec3(1.0f));
  this->Shader.SetFloat("alpha", 1.0f);

  for (std::size_t i = 0; i < level.brushes.size(); ++i) {
    if (world.doorOpen[i])
      continue;

    const level::Brush &brush = level.brushes[i];
    const glm::vec3 center = (brush.min + brush.max) * 0.5f;
    const glm::vec3 size = brush.max - brush.min;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
    model = glm::scale(model, size);
    this->Shader.SetMat4("model", model);
    this->Cube.Draw();
  } // end for

  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// -------------------------------------------------------------------------
void render::SceneRenderer::_DrawExit(const level::Level &level) const {
  if (!level.hasExit)
    return;

  // Transparent passes go last. Keep depth testing so walls occlude the exit,
  // but disable depth writes so it does not hide other transparent geometry.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  this->_DrawBox(level.exit.min, level.exit.max,
                 glm::vec3(0.2f, 1.0f, 0.3f), false, 0.35f);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

// eof - SceneRenderer.cpp
