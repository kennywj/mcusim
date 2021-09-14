cmake_minimum_required(VERSION 2.6)
#message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(esp8266)

include_directories(
	${CMAKE_SOURCE_DIR}/lwip/src/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_SOURCE_DIR}/lwip_port/freertos
	${CMAKE_SOURCE_DIR}/shell
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/include/mbedtls
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(esp8266 cobs.c comm.c crc16.c esp8266_if.c netconf.c)
