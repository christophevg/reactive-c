file(GLOB SRCS *.c)

add_library(reactive-lib ${SRCS})

target_include_directories(reactive-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
