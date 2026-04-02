cmake_minimum_required(VERSION 3.20)
project(debloat-helper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(debloat-helper
        src/main.cpp
        src/process.cpp
        src/service.cpp
        src/filesystem.cpp
        src/registry.cpp
)

target_compile_definitions(debloat-helper PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
)

target_link_libraries(debloat-helper PRIVATE
        advapi32    # SCM (services), registry, token privileges
        kernel32    # file/process APIs
)

# Output directly to build/ as debloat-helper.exe
set_target_properties(debloat-helper PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)
