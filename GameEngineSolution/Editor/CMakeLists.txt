cmake_minimum_required(VERSION 3.20)
project(Editor)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create executable (Console application for now, or Windows if preferred)
add_executable(Editor ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(Editor PRIVATE include)

# Link Engine
target_link_libraries(Editor PRIVATE Engine)

# Preprocessor definitions
target_compile_definitions(Editor PRIVATE 
    UNICODE 
    _UNICODE 
    WIN32 
    _CONSOLE
)

if(MSVC)
    target_compile_options(Editor PRIVATE /W3 /MP)
endif()
