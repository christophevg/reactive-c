file(GLOB SRCS *.c)

add_library(reactive-c ${SRCS})

target_include_directories(reactive-c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
