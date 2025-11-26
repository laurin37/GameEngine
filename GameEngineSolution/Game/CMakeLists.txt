cmake_minimum_required(VERSION 3.20)
project(Game)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create executable (Windows application)
add_executable(Game WIN32 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(Game PRIVATE include)

# Link Engine
target_link_libraries(Game PRIVATE Engine)

# Preprocessor definitions
target_compile_definitions(Game PRIVATE 
    UNICODE 
    _UNICODE 
    WIN32 
    _WINDOWS
)

# Copy Assets to output directory
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../Assets
    ${CMAKE_BINARY_DIR}/Assets
)

if(MSVC)
    target_compile_options(Game PRIVATE /W3 /MP)
endif()
