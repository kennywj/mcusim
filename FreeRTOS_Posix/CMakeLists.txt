
cmake_minimum_required(VERSION 2.6)
message(" **** Currently path of CMakeLists = " ${CMAKE_CURRENT_SOURCE_DIR})

project(rtos)

include_directories(
	${CMAKE_SOURCE_DIR}/lwip_port 
	${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS_Kernel/include
	${CMAKE_CURRENT_SOURCE_DIR}/Common_Demo/include
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(rtos FileIO/fileIO.c 
	AsyncIO/AsyncIO.c AsyncIO/AsyncIOSerial.c AsyncIO/AsyncIOSocket.c AsyncIO/PosixMessageQueueIPC.c
	FreeRTOS_Kernel/croutine.c FreeRTOS_Kernel/list.c FreeRTOS_Kernel/queue.c FreeRTOS_Kernel/tasks.c 
	FreeRTOS_Kernel/portable/GCC/Posix/port.c FreeRTOS_Kernel/portable/MemMang/heap_3.c 
	Common_Demo/Minimal/blocktim.c Common_Demo/Minimal/countsem.c 
	Common_Demo/Minimal/crflash.c Common_Demo/Minimal/crhook.c 
	Common_Demo/Minimal/GenQTest.c Common_Demo/Minimal/QPeek.c Common_Demo/Minimal/recmutex.c 
	ParTest/ParTest.c)