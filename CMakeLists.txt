cmake_minimum_required(VERSION 3.20)
project(Renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies via vcpkg ----
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)


# ---- Collect source files ----
file(GLOB_RECURSE RENDERER_SRC
    Renderer/src/*.cpp
    Renderer/header/*.h
    main.cpp
)

add_executable(Renderer ${RENDERER_SRC})

# ---- Add include directories ----
# (your own headers)
target_include_directories(Renderer PRIVATE
    Renderer/header
)
target_include_directories(Renderer PRIVATE
    ${VCPKG_ROOT}/installed/x64-windows/include
)

# ---- Link external libraries ----
target_link_libraries(Renderer PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
)

# ---- Copy shaders next to executable after build ----
add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:Renderer>/shaders
)
# ---- Copy textures next to executable after build ----
add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/textures
            $<TARGET_FILE_DIR:Renderer>/textures
)
# ---- Copy models next to executable after build ----
add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/models
            $<TARGET_FILE_DIR:Renderer>/models
)