cmake_minimum_required(VERSION 2.8)
project(PACKET_TEST)

include_directories("../../src")
include_directories("../../../fsm/inc")

set(SOURCES main.c ../../src/danish.c
	../../src/danish_link.c
	../../../fsm/src/delay.c)

add_executable(PACKET_TEST ${SOURCES})
	

