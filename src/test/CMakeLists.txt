# TESTING

enable_testing()

cmake_policy(SET CMP0003 OLD)
cmake_policy(SET CMP0023 OLD)

add_custom_target(build-test)
SET(ENV{CTEST_OUTPUT_ON_FAILURE} TRUE)

# create executables of all example files in the current directory
file(GLOB TESTS *.c)
foreach(test ${TESTS})
  message(STATUS "Adding test ${test}")
  get_filename_component(name ${test} NAME_WE)
  add_executable(${name} ${test})
  target_link_libraries(${name} LINK_PUBLIC reactive-c)
  target_link_libraries(${name} ${GLIB2_LIBRARIES})
  add_test(NAME ${name} COMMAND ${name})
  add_dependencies(build-test ${name})
endforeach()

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --verbose
)
