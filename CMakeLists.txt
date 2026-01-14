cmake_minimum_required(VERSION 3.20)
project(Renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies via vcpkg ----
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(fastGltf CONFIG REQUIRED)
find_package(OpenMP REQUIRED)


# ---- Collect source files ----
file(GLOB_RECURSE RENDERER_SRC

    	Renderer/Vulkan/*.cpp
	Renderer/Renderer/*.cpp
	Renderer/Platform/*.cpp
    	Renderer/Vulkan/*.h
	Renderer/Renderer/*.h
	Renderer/Platform/*.h

    	Engine/Core/*.cpp
	Engine/Scene/*.cpp
	Engine/Resources/*.cpp
    	Engine/Loaders/*.cpp
    	Engine/Core/*.h
	Engine/Scene/*.h
	Engine/Resources/*.h
    	Engine/Loaders/*.h

    	main.cpp
)

add_executable(Renderer ${RENDERER_SRC})

# ---- Add include directories ----
# (your own headers)
target_include_directories(Renderer PRIVATE
    Renderer/header
$<TARGET_PROPERTY:fastgltf::fastgltf,INTERFACE_INCLUDE_DIRECTORIES>
)


# ---- Link external libraries ----
target_link_libraries(Renderer PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
    fastgltf::fastgltf
     ${VMA_INCLUDE_DIRS}
	OpenMP::OpenMP_CXX
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