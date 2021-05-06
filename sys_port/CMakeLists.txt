
message("CURRENT_DIR= ${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
	${CMAKE_SOURCE_DIR}
)
# Create a library called "app" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (port STATIC freertos/network.c freertos/sys_arch.c freertos/ethernetif.c)

# Make sure the compiler can find include files for our app library
# when other libraries or executables link to app
target_include_directories(port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})