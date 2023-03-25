cmake_minimum_required(VERSION 2.8)
project(PACKET_TEST)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCES "main.c"
	"${DANISH_SOURCES}"
	"${FSM_SOURCES}")

add_executable(PACKET_TEST ${SOURCES})
	

