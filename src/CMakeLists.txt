## =========================================================================
## Arena FPS - build
## =========================================================================

cmake_minimum_required(VERSION 3.16)
project(arena_fps LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  )
endif()

# System dependencies (window + OpenGL)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Vendored dependencies: GLAD (loader) and GLM (math)
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(
  glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)
target_link_libraries(glad PUBLIC OpenGL::GL ${CMAKE_DL_LIBS})

add_library(glm INTERFACE)
target_include_directories(
  glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm
)

# tinyobjloader: header-only, the implementation macro lives in ObjLoader.cpp
add_library(tinyobjloader INTERFACE)
target_include_directories(
  tinyobjloader SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader
)

# Game executable
add_executable(
  arena
  main.cpp
  gl/Shader.cpp
  gl/Mesh.cpp
  gl/Camera.cpp
  level/ObjLoader.cpp
)
target_include_directories(arena PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(arena PRIVATE glad glm tinyobjloader glfw OpenGL::GL)

## eof - CMakeLists.txt
