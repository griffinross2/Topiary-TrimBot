cmake_minimum_required(VERSION 3.22)
project(Plant-Detect)

include(FetchContent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif(UNIX)

# Executable
add_executable(${PROJECT_NAME})

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

FetchContent_GetProperties(glfw
    POPULATED glfw_POPULATED
    SOURCE_DIR glfw_SOURCE_DIR
    BINARY_DIR glfw_BINARY_DIR
)

if(glfw_POPULATED)
    message(STATUS "glfw has been populated.")
    message(STATUS "Source directory: ${glfw_SOURCE_DIR}")
    message(STATUS "Binary directory: ${glfw_BINARY_DIR}")
endif()

# IMGUI
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends)
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_glfw.cpp)

# OpenCV
find_package(OpenCV REQUIRED)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
)

FetchContent_MakeAvailable(glad)

FetchContent_GetProperties(glad
    POPULATED glad_POPULATED
    SOURCE_DIR glad_SOURCE_DIR
    BINARY_DIR glad_BINARY_DIR
)

if(glad_POPULATED)
    message(STATUS "glad has been populated.")
    message(STATUS "Source directory: ${glad_SOURCE_DIR}")
    message(STATUS "Binary directory: ${glad_BINARY_DIR}")
endif()

# Generate glad
add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
glad_add_library(glad_gl_core_45 REPRODUCIBLE API gl:core=4.5)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    opengl32
    glad_gl_core_45
    ${OpenCV_LIBS}
)

# Headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Source files
add_subdirectory(src)

# Copy in fonts
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
    "${CMAKE_CURRENT_BINARY_DIR}/fonts"
    COMMENT "Copying in fonts"
)

# Copy in images
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/images"
    "${CMAKE_CURRENT_BINARY_DIR}/images"
    COMMENT "Copying in images"
)