cmake_minimum_required(VERSION 2.6)
message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(app)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip/src/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_SOURCE_DIR}/lwip_port/freertos
	${CMAKE_SOURCE_DIR}/FreeRTOS_Posix
	${CMAKE_SOURCE_DIR}/FreeRTOS_Posix/FreeRTOS_Kernel/include
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/include/mbedtls
	${CMAKE_SOURCE_DIR}/shell
	${CMAKE_SOURCE_DIR}/fatfs/src
	${CMAKE_SOURCE_DIR}/fatfs/port
	${CMAKE_CURRENT_SOURCE_DIR}/json
	${CMAKE_CURRENT_SOURCE_DIR}/gnss
	${CMAKE_CURRENT_SOURCE_DIR}/duktape
	${CMAKE_CURRENT_SOURCE_DIR}/http-parser)

add_library(app STATIC 
	app.c tcpecho.c ping.c cmdnet.c cmdduktape.c cmdfs.c http_test.c tftps.c https.c 
	pppd.c xmodem.c camera.c cmdmd5.c
	http-parser/http_parser.c http-parser/contrib/url_parser.c http-parser/contrib/parsertrace.c
	duktape/duktape.c 
	json/tiny-json.c json/json-maker.c json/cmd_json.c
	gnss/gnss.c gnss/minmea.c gnss/allystar.c)