cmake_minimum_required(VERSION 2.6)
message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(fatfs)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/port)

add_library(fatfs STATIC src/diskio.c src/ff.c 
	src/option/cc950.c src/option/syscall.c src/option/unicode.c 
	port/fattime.c port/fs_port.c port/ramdisk.c)