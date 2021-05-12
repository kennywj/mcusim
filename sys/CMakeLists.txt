cmake_minimum_required(VERSION 2.6)
message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(rtos)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_SOURCE_DIR}/shell
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(sys util.c sys.c fifo.c cmd_fifo.c)