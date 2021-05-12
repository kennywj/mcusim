include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/include/mbedtls
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DU2W_SIM -DMBEDTLS_CONFIG_FILE='<u2w_config.h>'")

# Create a library called "port" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (tlstest STATIC 
	${CMAKE_SOURCE_DIR}/mbedtls-2.6.0/programs/test/selftest.c)