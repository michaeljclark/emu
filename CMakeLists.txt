cmake_minimum_required(VERSION 3.12)

project(emu)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(emu/include)
add_library(emu_lib emu/lib/sys.cc emu/lib/uart.cc emu/lib/ich.cc)
add_executable(emu emu/emu/emu.cc)
target_link_libraries(emu PRIVATE emu_lib)

file(GLOB test_rules RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*/rules.mk)
foreach(test_rule ${test_rules})
  get_filename_component(test_dir ${test_rule} DIRECTORY)
  get_filename_component(test_name ${test_dir} NAME)
  add_executable(test_${test_name} ${test_dir}/test.cc)
  target_link_libraries(test_${test_name} PRIVATE emu_lib)
endforeach()

include(FindPythonInterp)
add_custom_target(
 always_run_target ALL
 COMMAND ${PYTHON_EXECUTABLE} "scripts/firmware.py"
 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
