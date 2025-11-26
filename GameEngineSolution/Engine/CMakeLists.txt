cmake_minimum_required(VERSION 3.20)
project(Engine)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create library
add_library(Engine STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(Engine PUBLIC include)

# Link libraries
target_link_libraries(Engine PUBLIC 
    d3d11 
    dxgi 
    d3dcompiler 
    dxguid
)

# Preprocessor definitions
target_compile_definitions(Engine PRIVATE 
    UNICODE 
    _UNICODE 
    WIN32 
    _WINDOWS
)

if(MSVC)
    target_compile_options(Engine PRIVATE /W3 /MP)
endif()
