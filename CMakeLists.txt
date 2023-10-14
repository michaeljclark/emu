cmake_minimum_required(VERSION 3.12)

project(emu)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# macros for compiler and linker flags
#
macro(add_compiler_flag)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
   set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${ARGN}")
endmacro(add_compiler_flag)

macro(add_linker_flag)
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${ARGN}")
endmacro(add_linker_flag)

#
# helper macro for logging variables
#
macro(log_boolean)
  set(spaces "                                                  ")
  string(LENGTH ${ARGV0} slen)
  math(EXPR plen "50 - ${slen}")
  string(SUBSTRING ${spaces} 0 ${plen} spaces)
  if (${${ARGV0}})
    message("${ARGV0}${spaces}: true")
  else()
    message("${ARGV0}${spaces}: false")
  endif()
endmacro()

#
# check for MSVC compiler flags and intrinsics
#

# check for MSVC /WX flag and enable for flag tests
check_c_compiler_flag(/WX has_cc_flag_WX)
if (has_cc_flag_WX)
  # MSVC will treat warnings as errors for flag tests
  list(APPEND CMAKE_REQUIRED_FLAGS "/WX")
endif()

# check for MSVC /volatile:iso then enable
check_c_compiler_flag(/volatile:iso has_cc_flag_volatile_iso)
if (has_cc_flag_volatile_iso)
  # MSVC will define _ISO_VOLATILE 1
  add_compiler_flag(/volatile:iso)
  list(APPEND CMAKE_REQUIRED_FLAGS "/volatile:iso")
endif()

# check for MSVC interlocked intrinsics
check_c_source_compiles("#include <intrin.h>
int main() { long v = 0; _InterlockedExchange(&v, 0); }" has_fn_intrin_h_interlocked_exchange)
if(has_fn_intrin_h_interlocked_exchange)
  add_definitions(-DHAVE_INTRIN_INTERLOCKED_EXCHANGE)
endif()

# check for C11 standard atomics
check_c_source_compiles("#include <stdatomic.h>
int main() { _Atomic int v; return atomic_exchange(&v,0); }" has_fn_std_atomic_h_atomic_exchange)

# enable C11 atomic compat
if(NOT has_fn_std_atomic_h_atomic_exchange)
  set(requires_compat_stdatomic_h 1)
endif()

# log configuration results
log_boolean(has_cc_flag_volatile_iso)
log_boolean(has_fn_intrin_h_interlocked_exchange)
log_boolean(has_fn_std_atomic_h_atomic_exchange)
log_boolean(requires_compat_stdatomic_h)

#
# emu
#
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

option(LIBUV_BUILD_SHARED "Build shared lib" OFF)
add_subdirectory(third_party/libuv)
include_directories(third_party/libuv/include)
install(EXPORT libuvConfig DESTINATION lib)

include_directories(emu/include common/include)
add_library(emu_lib emu/lib/sys.cc emu/lib/uart.cc emu/lib/ich.cc)
add_executable(emu emu/emu/emu.cc)
target_link_libraries(emu PRIVATE emu_lib uv_a)

add_subdirectory(guest)
add_subdirectory(common)

include(FindPythonInterp)
add_custom_target(
 always_run_target ALL
 COMMAND ${PYTHON_EXECUTABLE} "scripts/firmware.py"
 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
