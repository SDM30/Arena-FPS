// =========================================================================
// Scene renderer: draws the level, world entities and debug overlays
// =========================================================================
#ifndef __render__SceneRenderer__h__
#define __render__SceneRenderer__h__

#include <glm/glm.hpp>

#include <gl/Mesh.h>
#include <gl/Shader.h>

#include <string>

namespace game {
struct World;
} // namespace game

namespace gl {
class Camera;
} // namespace gl

namespace level {
struct Level;
} // namespace level

namespace render {
struct Options {
  bool showCollision = false;
};

class SceneRenderer {
public:
  SceneRenderer() = default;
  ~SceneRenderer() = default;

  // Rule of five
  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&) = delete;
  SceneRenderer &operator=(SceneRenderer &&) = delete;

  /**
   * Builds the shader and uploads the cube and level meshes. The GL context
   * must be current. Returns false and fills "error" on the first failure.
   */
  bool Initialize(const level::Level &level, std::string *error);

  /**
   * Uploads a new level mesh without rebuilding the shader or cube.
   */
  bool LoadLevel(const level::Level &level, std::string *error);

  /**
   * Draws one frame from read-only level and world state. Does not update
   * simulation state and does not swap buffers.
   */
  void Draw(const gl::Camera &camera, const level::Level &level,
            const game::World &world, int width, int height,
            const Options &options) const;

private:
  void _DrawBox(const glm::vec3 &min, const glm::vec3 &max,
                const glm::vec3 &color, bool wireframe = false,
                float alpha = 1.0f) const;
  void _DrawTraps(const level::Level &level) const;
  void _DrawDoors(const level::Level &level, const game::World &world) const;
  void _DrawKeys(const level::Level &level, const game::World &world) const;
  void _DrawEnemies(const game::World &world) const;
  void _DrawBrushes(const level::Level &level,
                    const game::World &world) const;
  void _DrawExit(const level::Level &level) const;

  gl::Shader Shader;
  gl::Mesh Cube;
  gl::Mesh LevelMesh;
};
} // namespace render

#endif // __render__SceneRenderer__h__

// eof - SceneRenderer.h
