# EXAMPLES

cmake_policy(SET CMP0003 OLD)

# top-level run-target, run-targets for each executable are added below
add_custom_target(examples)

# create executables of all example files in the current directory
file(GLOB EXAMPLES *.c)
foreach(example ${EXAMPLES})
  message(STATUS "Adding example ${example}")
  get_filename_component(name ${example} NAME_WE)
  add_executable(${name} ${example})
  target_link_libraries(${name} LINK_PUBLIC reactive-c)
  target_link_libraries(${name} LINK_PUBLIC unit)
  add_custom_target(run_${name}
    DEPENDS ${name}
    COMMENT "Running ${name}..."
    COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
  # and make it a dependency of the generic run target
  add_dependencies(examples "run_${name}")
endforeach()
