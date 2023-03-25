cmake_minimum_required(VERSION 2.8)
project(COMM_PROTOCOL_TEST)

include_directories("../osfsm/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

set(FSM_SOURCES	"${CMAKE_CURRENT_SOURCE_DIR}/../osfsm/src/delay.c")

set(DANISH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/danish.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/danish_link.c")

add_subdirectory("test/link")
add_subdirectory("test/packet")
