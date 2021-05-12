cmake_minimum_required(VERSION 2.6)
message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(shell)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(shell STATIC cmd.c console.c parser.c)