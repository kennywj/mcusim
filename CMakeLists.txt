cmake_minimum_required (VERSION 2.6)
#message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
ENDIF()
#message("Software build mode " ${CMAKE_BUILD_TYPE})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -Wno-pointer-sign -fPIC")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
ENDIF ()
#message("C compile options" ${CMAKE_C_FLAGS})

include_directories(
	${CMAKE_SOURCE_DIR} 
	lwip_port
	FreeRTOS_Posix
	FreeRTOS_Posix/FreeRTOS_Kernel/include
	FreeRTOS_Posix/Common_Demo/include)

project(ncusim)

add_subdirectory(FreeRTOS_Posix)
add_subdirectory(sys)
add_subdirectory(shell)
add_subdirectory(fatfs)
add_subdirectory(lwip_port)
add_subdirectory(mbedtls-2.6.0)
add_subdirectory(app)
add_subdirectory(esp8266)

add_executable(mcusim main.c)

# duplicate add librarys like lwip rtos to solve recursive dependence issues
target_link_libraries(mcusim shell sys app sys esp8266 fatfs lwip mbedtls rtos pthread m)