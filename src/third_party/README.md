# Vendored dependencies

| Directory | Origin | Version | License |
|---|---|---|---|
| `glm/` | https://github.com/g-truc/glm | 1.0.1 | MIT (`glm/copying.txt`) |
| `glad/` | Generated with glad2 (`--api gl:core=3.3 --extensions "" c --loader`) | OpenGL 3.3 core | WTFPL / CC0-1.0 / Apache-2.0 (stated in `glad/src/gl.c`) |
| `tinyobjloader/` | https://github.com/tinyobjloader/tinyobjloader (release `v1.0.6`) | 1.0.6 | MIT (`tinyobjloader/LICENSE`) |

GLFW and OpenGL are linked from the system (`find_package(glfw3)` and
`find_package(OpenGL)`). tinyobjloader is header-only: the
`TINYOBJLOADER_IMPLEMENTATION` macro is defined only in `level/ObjLoader.cpp`.
