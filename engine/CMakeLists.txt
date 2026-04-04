cmake_minimum_required(VERSION 3.25)

project(Engine LANGUAGES CXX)

add_library(Engine STATIC)

target_sources(Engine PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/libs.cpp
)

file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)
target_sources(Engine PRIVATE ${ENGINE_SRC})

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(Engine PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Vulkan SDK
find_package(Vulkan REQUIRED)

# GLFW
add_subdirectory(libs/glfw)

# Link GLFW
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)