cmake_minimum_required(VERSION 3.16)

project(graphic-samples)

set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH TRUE)

option(BUILD_TESTS "build tests" OFF)

include_directories(
  ${CMAKE_SOURCE_DIR}/opengl
  ${CMAKE_SOURCE_DIR}/glut
  ${CMAKE_SOURCE_DIR}/x11
  ${CMAKE_SOURCE_DIR}/xcb
)

add_library(common
  STATIC
  opengl/draw_circle.c
  opengl/get_version.c
  opengl/draw_bezier_surface.c
  opengl/draw_sphere.c
  opengl/load_file_by_assimp.c
)

add_executable(x11test x11/x11-for-opengl.c)
target_link_libraries(x11test
  common
  GL
  GLU
  assimp
  xcb
  X11-xcb
  X11
  m
  dl
)

add_executable(xcbtest xcb/xcb-for-opengl.c)
target_link_libraries(xcbtest
  common
  GL
  GLU
  assimp
  xcb
  X11-xcb
  X11
  m
  dl
)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
