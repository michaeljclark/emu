cmake_minimum_required(VERSION 3.12)

project(emu)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(emu/include common/include)
add_library(emu_lib emu/lib/sys.cc emu/lib/uart.cc emu/lib/ich.cc)
add_executable(emu emu/emu/emu.cc)
target_link_libraries(emu PRIVATE emu_lib)

add_subdirectory(guest)
add_subdirectory(common)

include(FindPythonInterp)
add_custom_target(
 always_run_target ALL
 COMMAND ${PYTHON_EXECUTABLE} "scripts/firmware.py"
 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
