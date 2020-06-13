cmake_minimum_required(VERSION 3.13)  # CMake version check
project(ass4)               # Create project "simple_example"
set(CMAKE_BUILD_TYPE Debug)
# Add main.c file of project root directory as source file
set(SOURCE_FILES_MAPPER mapper.c networking.c)
set(SOURCE_FILES_ROC roc.c networking.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(SOURCE_FILES_CONTROL control.c networking.c)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(mapper ${SOURCE_FILES_MAPPER})
add_executable(control ${SOURCE_FILES_CONTROL})
add_executable(roc ${SOURCE_FILES_ROC})

set_property(TARGET roc PROPERTY C_STANDARD 99)
set_property(TARGET mapper PROPERTY C_STANDARD 99)
set_property(TARGET control PROPERTY C_STANDARD 99)
