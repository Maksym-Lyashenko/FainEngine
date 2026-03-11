cmake_minimum_required(VERSION 3.25)

project (FainEngine LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/Game.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

add_subdirectory(engine)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# # Vulkan SDK
# find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Engine stb_image ktx)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()