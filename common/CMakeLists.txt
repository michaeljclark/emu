cmake_minimum_required(VERSION 3.12)

file(GLOB crypto_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} crypto/*.c)
add_library(crypto ${crypto_src})

file(GLOB common_rules RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*/rules.mk)
foreach(test_rule ${common_rules})
  get_filename_component(test_dir ${test_rule} DIRECTORY)
  get_filename_component(test_name ${test_dir} NAME)
  add_executable(test_${test_name} ${test_dir}/test.c)
  target_link_libraries(test_${test_name} crypto uv_a)
endforeach()
