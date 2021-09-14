cmake_minimum_required(VERSION 2.6)
#message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(app)

enable_language(ASM)
set_property(SOURCE vgg19_para.S PROPERTY LANGUAGE ASM)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/lwip/src/include
	${CMAKE_SOURCE_DIR}/lwip_port
	${CMAKE_SOURCE_DIR}/lwip_port/freertos
	${CMAKE_SOURCE_DIR}/FreeRTOS_Posix
	${CMAKE_SOURCE_DIR}/FreeRTOS_Posix/FreeRTOS_Kernel/include
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/include/mbedtls
	${CMAKE_SOURCE_DIR}/shell
	${CMAKE_SOURCE_DIR}/nn/inc
	${CMAKE_SOURCE_DIR}/fatfs/src
	${CMAKE_SOURCE_DIR}/fatfs/port
	${CMAKE_CURRENT_SOURCE_DIR}/json
	${CMAKE_CURRENT_SOURCE_DIR}/gnss
	${CMAKE_CURRENT_SOURCE_DIR}/duktape
	${CMAKE_CURRENT_SOURCE_DIR}/http-parser
	${CMAKE_CURRENT_SOURCE_DIR}/nn
	${CMAKE_CURRENT_SOURCE_DIR}/nn/vgg_wb_bin/)

add_library(app STATIC 
	app.c tcpecho.c ping.c cmdnet.c cmdduktape.c cmdfs.c http_test.c tftps.c https.c 
	pppd.c xmodem.c camera.c cmdmd5.c cmdsha1.c
	http-parser/http_parser.c http-parser/contrib/url_parser.c http-parser/contrib/parsertrace.c
	duktape/duktape.c 
	json/tiny-json.c json/json-maker.c json/cmd_json.c
	gnss/gnss.c gnss/minmea.c gnss/allystar.c
	nn/cmd_cifar10.c nn/cmdconv.c nn/cmdpool.c nn/cmd_vgg19.c nn/vgg19_para.S)