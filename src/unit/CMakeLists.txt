file(GLOB SRCS *.c)

add_library(unit ${SRCS})

target_include_directories(unit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
