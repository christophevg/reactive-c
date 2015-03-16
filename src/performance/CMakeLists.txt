# PERFORMANCE

add_custom_target(perf)

file(GLOB PERF_TESTS *.c)
foreach(perf ${PERF_TESTS})
  message(STATUS "Adding performance test ${perf}")
  get_filename_component(name ${perf} NAME_WE)
  add_executable(${name} ${perf})
  target_link_libraries(${name} LINK_PUBLIC reactive-c)
  target_link_libraries(${name} LINK_PUBLIC reactive-lib)
  add_custom_target(run_${name}
    DEPENDS ${name}
    COMMENT "Running performance test ${name}..."
    COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
  # and make it a dependency of the generic run target
  add_dependencies(perf "run_${name}")
endforeach()
