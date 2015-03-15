project (reactive)
cmake_minimum_required(VERSION 2.4)

file(GLOB REACTIVE_LIBS reactive-c/*.c)
SET(LIBS ${REACTIVE_LIBS} "unit/test.c" "unknown/unknown.c" "lib/time.c")

message(STATUS "Build Configuration Summary:")
message(STATUS "  CMAKE_BUILD_TYPE        : ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG   : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "  CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")

# ok for clang and gcc, good enough for now
add_compile_options(-std=c99)
add_compile_options(-Wall)
add_compile_options(-pedantic)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_compile_options(-lrt)
endif()

# we're using GLib for its test harness
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})

include_directories(.)

# EXAMPLES

# top-level run-target, run-targets for each executable are added below
add_custom_target(examples)

# create executables of all example files in the current directory
file(GLOB EXAMPLES examples/*.c)
foreach(example ${EXAMPLES})
  message(STATUS "Adding example ${example}")
  get_filename_component(name ${example} NAME_WE)
  add_executable(${name} ${example} ${LIBS})
  add_custom_target(run_${name}
    DEPENDS ${name}
    COMMENT "Running ${name}..."
    COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
  # and make it a dependency of the generic run target
  add_dependencies(examples "run_${name}")
endforeach()

enable_testing()

# TESTING

add_custom_target(build-test)
SET(ENV{CTEST_OUTPUT_ON_FAILURE} TRUE)

# create executables of all example files in the current directory
file(GLOB TESTS test/*.c)
foreach(test ${TESTS})
  message(STATUS "Adding test ${test}")
  get_filename_component(name ${test} NAME_WE)
  add_executable(${name} ${test} ${LIBS})
  target_link_libraries(${name} ${GLIB2_LIBRARIES})
  add_test(NAME ${name} COMMAND ${name})
  add_dependencies(build-test ${name})
endforeach()

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --verbose
)

# PERFORMANCE

add_custom_target(perf)

file(GLOB PERF_TESTS performance/*.c)
foreach(perf ${PERF_TESTS})
  message(STATUS "Adding performance test ${perf}")
  get_filename_component(name ${perf} NAME_WE)
  add_executable(${name} ${perf} ${LIBS})
  target_link_libraries(${name} ${GLIB2_LIBRARIES})
  add_custom_target(run_${name}
    DEPENDS ${name}
    COMMENT "Running performance test ${name}..."
    COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
  # and make it a dependency of the generic run target
  add_dependencies(perf "run_${name}")
endforeach()
