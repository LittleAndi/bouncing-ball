cmake_minimum_required(VERSION 3.15)
project(BouncingBallEngine LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find vcpkg (if installed globally) OR use manifest
# If you ran `vcpkg integrate install`, this works automatically.
# If not, we'll use the manifest below.

# Find packages from vcpkg
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h" REQUIRED)

# Add executable
add_executable(BouncingBallEngine
    src/main.cpp
    src/shader.cpp
)

# Include directories
target_include_directories(BouncingBallEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    ${MINIAUDIO_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(BouncingBallEngine PRIVATE
    glad::glad
    glfw
    glm::glm-header-only
)

# Copy shaders next to the executable after each build
add_custom_command(TARGET BouncingBallEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:BouncingBallEngine>/shaders
)