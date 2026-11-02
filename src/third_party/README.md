# Dependencias vendorizadas

| Directorio | Origen | Versión | Licencia |
|---|---|---|---|
| `glm/` | https://github.com/g-truc/glm | 1.0.1 | MIT (`glm/copying.txt`) |
| `glad/` | Generado con glad2 (`--api gl:core=3.3 --extensions "" c --loader`) | OpenGL 3.3 core | WTFPL / CC0-1.0 / Apache-2.0 (`glad/src/gl.c` lo indica) |
| `tinyobjloader/` | https://github.com/tinyobjloader/tinyobjloader (release `v1.0.6`) | 1.0.6 | MIT (`tinyobjloader/LICENSE`) |

GLFW y OpenGL se enlazan desde el sistema (`find_package(glfw3)` y
`find_package(OpenGL)`). tinyobjloader es header-only: la macro
`TINYOBJLOADER_IMPLEMENTATION` se define únicamente en `level/ObjLoader.cpp`.
