project (reactive)
cmake_minimum_required(VERSION 2.4)

message(STATUS "Build Configuration Summary:")
message(STATUS "  CMAKE_BUILD_TYPE        : ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG   : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "  CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")

# ok for clang and gcc, good enough for now
add_compile_options(-std=c99)
add_compile_options(-Wall)
add_compile_options(-pedantic)

# we're using GLib for its test harness
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})

include_directories(.)

add_subdirectory(reactive-c)
add_subdirectory(lib)
add_subdirectory(unit)
add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(performance)
