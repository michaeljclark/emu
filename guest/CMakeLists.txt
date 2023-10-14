cmake_minimum_required(VERSION 3.12)

file(GLOB test_rules RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*/rules.mk)
foreach(test_rule ${test_rules})
  get_filename_component(test_dir ${test_rule} DIRECTORY)
  get_filename_component(test_name ${test_dir} NAME)
  add_executable(test_${test_name} ${test_dir}/test.cc)
  target_link_libraries(test_${test_name} PRIVATE emu_lib uv_a)
endforeach()
