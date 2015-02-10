project (unknown)
cmake_minimum_required(VERSION 2.4)

add_executable(demo demo.c unknown.c ../unit/test.c)

add_custom_target(run
  DEPENDS demo
  COMMENT "Running demo..."
  COMMAND demo
  WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
