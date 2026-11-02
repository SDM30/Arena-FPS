# Arena FPS

FPS de arena en primera persona construido sobre OpenGL 3.3 core.

## Stack

| Área | Librería |
|---|---|
| Ventana / input | GLFW 3.4 |
| OpenGL | 3.3 core |
| Carga de funciones GL | GLAD |
| Matemática | GLM 1.0.1 |
| Geometría `.obj` | tinyobjloader 1.0.6 |
| Build | CMake ≥ 3.16, C++20 |

## Requisitos

- Compilador con C++20.
- CMake ≥ 3.16.
- OpenGL y GLFW 3.3+ instalados en el sistema.

En Debian/Ubuntu:

```bash
sudo apt install cmake g++ libglfw3-dev
```

GLM, GLAD y tinyobjloader están vendorizados en `src/third_party/`, así que no
hace falta instalarlos ni descargarlos.

## Compilación y ejecución

```bash
cmake -S src -B build
cmake --build build -j
./build/arena
```

## Controles

| Acción | Entrada |
|---|---|
| Mirar | Ratón |
| Liberar el cursor | `Esc` |
| Recapturar el cursor | Clic izquierdo |

El movimiento con WASD, el salto y el disparo se añaden en los hitos
siguientes.

## Estructura

```
src/
  main.cpp        Arranque, ventana, contexto GL y loop
  gl/             Shader, malla (VAO/VBO/EBO) y cámara
  physics/        Mundo de colisión y raycasts
  game/           Jugador, armas, enemigos y reglas
  level/          Carga de OBJ con tinyobjloader y capa que define el nivel
  ui/             Integración de Dear ImGui
  third_party/    GLM, GLAD y tinyobjloader vendorizados
levels/           Geometría y metadata de los niveles
```

## Dependencias externas/vendorizadas

Ver `src/third_party/README.md` para el origen, la versión y la licencia de
cada dependencia.

