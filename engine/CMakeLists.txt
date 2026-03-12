cmake_minimum_required(VERSION 3.25)

project(Engine LANGUAGES C CXX)

add_library(Engine STATIC)

target_sources(Engine PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/libs.cpp
)

file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)
target_sources(Engine PRIVATE ${ENGINE_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/libs/tracy/public/TracyClient.cpp)

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(Engine PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Vulkan SDK
find_package(Vulkan REQUIRED)

if (TARGET Vulkan::Vulkan)
  target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
endif()

# GLFW
add_subdirectory(libs/glfw)

# Taskflow
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_PROFILER OFF CACHE BOOL "" FORCE)
set(TF_BUILD_CUDA OFF CACHE BOOL "" FORCE)
set(TF_BUILD_MODULES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/taskflow)

# gslang
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE) 
add_subdirectory(libs/glslang)

# ldrutils
add_subdirectory(libs/ldrutils) 

# stb_image
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)

# KTX
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(LIBKTX_VERSION_READ_ONLY    OFF CACHE BOOL "")
set(LIBKTX_VERSION_FULL          ON CACHE BOOL "")
set(LIBKTX_FEATURE_KTX1          ON CACHE BOOL "")
set(LIBKTX_FEATURE_KTX2          ON CACHE BOOL "")
set(LIBKTX_FEATURE_VK_UPLOAD    OFF CACHE BOOL "")
set(LIBKTX_FEATURE_GL_UPLOAD    OFF CACHE BOOL "")
set(LIBKTX_FEATURE_ETC_UNPACK    ON CACHE BOOL "")
add_subdirectory(libs/KTX/lib)

# vma
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vma/include)

# GLM
add_subdirectory(libs/glm)

# Assimp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
add_subdirectory(libs/assimp)

# ImGui
add_subdirectory(libs/imgui)

# Tracy
target_include_directories(Engine PUBLIC libs/tracy/public)

target_compile_definitions(Engine PUBLIC
    $<$<CONFIG:Debug>:TRACY_ENABLE>
    $<$<CONFIG:Debug>:ENG_ENABLE_TRACY>
)

# Link libs 
target_link_libraries(${PROJECT_NAME} PUBLIC
  glfw 
  Taskflow::Taskflow 
  glslang
  glslang-default-resource-limits
  LUtils
  stb_image
  ktx
  vma
  glm
  assimp
  imgui
 )

