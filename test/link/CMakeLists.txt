cmake_minimum_required(VERSION 2.8)
project(LINK_TEST)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCES 
	"main.c"	
	"${DANISH_SOURCES}"
	"${FSM_SOURCES}")

add_executable(LINK ${SOURCES})
	

